#include <stdlib.h>     /* realloc */
// QT
#include <QImage>
#include <QString>
#include <QFileInfo>
#include <QDebug>
#include <QTemporaryFile>
#include <QDir>
#include <QColor>
#include <QtEndian>

// DCMTK includes
#include <dcmtk/dcmimgle/dcmimage.h>
#include <dcmtk/dcmdata/dcfilefo.h>
#include <dcmtk/dcmdata/dcobject.h>
#include <dcmtk/dcmimgle/diutils.h>
#include <dcmtk/ofstd/ofbmanip.h>
#include <dcmtk/ofstd/ofstring.h>
#include <dcmtk/dcmdata/dcelem.h>
#include <dcmtk/dcmdata/dcdict.h>
#include <dcmtk/dcmdata/dcdicent.h>
#include <dcmtk/dcmimage/diregist.h>
#include <dcmtk/dcmdata/dcdeftag.h>

// local
#include "dicomprocessor.h"

DicomProcessor::DicomProcessor()
{
    this->dicomImage = NULL;
    this->dicomFile = NULL;
}

DicomProcessor::~DicomProcessor()
{
    if(this->dicomImage) delete this->dicomImage;
    if(this->dicomFile) delete this->dicomFile;
}

DcmTagKey DicomProcessor::parseTagKey(const char *tagName)
{
    unsigned int group = 0xffff;
    unsigned int elem = 0xffff;
    if (sscanf(tagName, "%x,%x", &group, &elem) != 2)
    {
        DcmTagKey tagKey;
        /* it is a name */
        const DcmDataDictionary &globalDataDict = dcmDataDict.rdlock();
        const DcmDictEntry *dicent = globalDataDict.findEntry(tagName);
        if (dicent == NULL) {
            tagKey = DCM_UndefinedTagKey;
        } else {
            tagKey = dicent->getKey();
        }
        dcmDataDict.unlock();
        return tagKey;
    } else     /* tag name has format "gggg,eeee" */
    {
        return DcmTagKey(group,elem);
    }
}

QString DicomProcessor::getMetadata(QString id)
{
    if(!this->dicomFile)
        return "dicomFileNotSupplied";

    //TODO use id
    OFString ofstr((QString("!")+id).toLocal8Bit());
    DcmElement *element;
    DcmTagKey tagKey = parseTagKey(id.toLocal8Bit());

    if (tagKey == DCM_UndefinedTagKey)
    {
        ofstr = "UndefinedTagKey";
    }
    else if (this->dicomFile->getDataset()->findAndGetElement(tagKey, element).good())
    {
      element->getOFString(ofstr, 0);
    }

    return QString(ofstr.c_str());
}

void DicomProcessor::load(QString filePath)
{
    const QFileInfo fileInfo(filePath);

    this->dicomFile = new DcmFileFormat();
    DcmObject *dset = this->dicomFile;
    OFCondition cond = this->dicomFile->loadFile(fileInfo.absoluteFilePath().toLocal8Bit());
    if (cond.bad())
    {
        throw QString("couldn't load DICOM input file");
    }

    this->dicomImage = new DicomImage(dset, (E_TransferSyntax)0);
}

/**
 * FIXME pixel_rep
 */
template<typename T>
void transPixeltoMono(double max_value, QColor pixel_color, Uint16 pixel_rep, Uint8* ptr, E_TransferSyntax xfer)
{
    int mono = (qGray(pixel_color.rgb()) * max_value) / 256;
    switch(xfer) {
    case EXS_LittleEndianImplicit:
    case EXS_LittleEndianExplicit:
        *((T*)ptr) = qToLittleEndian((T) mono);
        break;
    case EXS_BigEndianImplicit:
    case EXS_BigEndianExplicit:
        *((T*)ptr) = qToBigEndian((T) mono);
        break;
    default:
        *((T*)ptr) = (T) mono;
        break;
    }
}

void putPixel(double max_value, Uint16 bitsAllocated, Uint16 pixel_rep,
                              QColor pixel_color, Uint8* ptr, E_TransferSyntax xfer) {

    if(pixel_rep==0) {
        switch(bitsAllocated/8) {
            case 1:
            transPixeltoMono<quint8>(max_value, pixel_color, pixel_rep, ptr, xfer);
                break;
            case 2:
            transPixeltoMono<quint16>(max_value, pixel_color, pixel_rep, ptr, xfer);
                break;
            case 4:
            transPixeltoMono<quint32>(max_value, pixel_color, pixel_rep, ptr, xfer);
                break;
        }
    } else {
        switch(bitsAllocated/8) {
            case 1:
            transPixeltoMono<qint8>(max_value, pixel_color, pixel_rep, ptr, xfer);
                break;
            case 2:
            transPixeltoMono<qint16>(max_value, pixel_color, pixel_rep, ptr, xfer);
                break;
            case 4:
            transPixeltoMono<qint32>(max_value, pixel_color, pixel_rep, ptr, xfer);
                break;
        }
    }

}

/**
* @brief DicomProcessor::save
* @param filePath
* @param label
* @param x
* @param y
* @return
*/
bool DicomProcessor::save(QString filePath, QImage label, int x, int y)
{
    DcmDataset dsCopy = *(this->dicomFile->getDataset());
    E_TransferSyntax writeXfer = dsCopy.getOriginalXfer();
    unsigned long length;
    const Uint8* pixDataC;
    Uint8* pixData;

    int x_max = this->dicomImage->getWidth(), y_max = this->dicomImage->getHeight();

    if(x<0) x+=x_max-label.size().width();
    if(y<0) y+=y_max-label.size().height();

    Sint32 numberOfFrames = 1;

    dsCopy.findAndGetSint32(DCM_NumberOfFrames, numberOfFrames);
    if (numberOfFrames < 1) numberOfFrames = 1;

    Uint16 bitsStored, bitsAllocated, samples_per_pixe1=1, pixel_rep;
    dsCopy.findAndGetUint16(DCM_BitsAllocated, bitsAllocated);
    dsCopy.findAndGetUint16(DCM_BitsStored, bitsStored);
    dsCopy.findAndGetUint16(DCM_PixelRepresentation, pixel_rep);
    if(dsCopy.findAndGetUint16(DCM_SamplesPerPixel, samples_per_pixe1) != EC_Normal)
        samples_per_pixe1 = 1;

    dsCopy.findAndGetUint8Array(DCM_PixelData, pixDataC, &length);
    const unsigned short int pixel_size = (bitsAllocated / 8) * samples_per_pixe1;

    double color_min, color_max;
    this->dicomImage->getMinMaxValues(color_min, color_max);

    for (int i = 0; i < numberOfFrames; ++i) {
        pixData = (Uint8*) pixDataC;
        pixData += i * pixel_size * x_max * y_max;
        for(int cur_x=x; (cur_x<x_max) && (cur_x-x<label.size().width()); cur_x++) {
            for(int cur_y=y; (cur_y<y_max) && (cur_y-y<label.size().height()); cur_y++) {
                for(int sample_i=0; sample_i<samples_per_pixe1; sample_i++ ) {
                    Uint8* pixel_ptr = pixData + pixel_size * cur_x + pixel_size * cur_y * x_max + sample_i * (bitsAllocated / 8);

                    if(pixel_ptr < (pixDataC + length)) {
                        QColor pixel_color = label.pixel(cur_x-x, cur_y-y);

                        if(this->dicomImage->getPhotometricInterpretation() == EPI_Monochrome1) {
                            pixel_color = QColor::fromRgb(pixel_color.rgb()^0xFFFFFF);
                        }

                        putPixel( color_max, bitsAllocated, pixel_rep, pixel_color, pixel_ptr, writeXfer);
                    } else {
                        std::cout << "error!" << std::endl;
                    }
                }
            }
        }
    }

    dsCopy.saveFile(filePath.toLocal8Bit(), writeXfer);
    return true; //failure is not an option // FIXME
}

//------------------------------------------------------------------------------
void DicomProcessor::loadDicomImage(DicomImage* dicomImage)
{
  this->dicomImage = dicomImage;
  if (this->dicomImage)
    // Select first window defined in image. If none, compute min/max window as best guess.
    // Only relevant for monochrome.
    if (this->dicomImage->isMonochrome())
    {
        if (this->dicomImage->getWindowCount() > 0)
        {
          this->dicomImage->setWindow(0);
        }
        else
        {
          this->dicomImage->setMinMaxWindow(OFTrue /* ignore extreme values */);
        }
    }
}

//------------------------------------------------------------------------------
unsigned long DicomProcessor::frameCount() const
{
  if (this->dicomImage)
    {
    return this->dicomImage->getFrameCount();
    }
  return 0;
}

//------------------------------------------------------------------------------
QImage DicomProcessor::frame(int frame) const
{

  // this way of converting the dicom image to a qpixmap was adopted from some code from
  // the DCMTK forum, posted by Joerg Riesmayer, see http://forum.dcmtk.org/viewtopic.php?t=120
  QImage image;
  if ((this->dicomImage != NULL) && (this->dicomImage->getStatus() == EIS_Normal))
    {
    /* get image extension */
    const unsigned long width = this->dicomImage->getWidth();
    const unsigned long height = this->dicomImage->getHeight();
    QString header = QString("P5 %1 %2 255\n").arg(width).arg(height);
    const unsigned long offset = header.length();
    const unsigned long length = width * height + offset;
    /* create output buffer for DicomImage class */
    QByteArray buffer;
    buffer.append(header);
    buffer.resize(length);

    /* copy PGM header to buffer */

    if (this->dicomImage->getOutputData(static_cast<void *>(buffer.data() + offset), length - offset, 8, frame))
      {

      if (!image.loadFromData( buffer ))
        {
            throw QString("QImage couldn't created");
        }
      }
    }
  return image.convertToFormat(QImage::Format_ARGB32);
}

int DicomProcessor::getRepresentativeFrame()
{
    return this->dicomImage->getRepresentativeFrame();
}
