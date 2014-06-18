#include <stdlib.h>     /* realloc */
// QT
#include <QImage>
#include <QString>
#include <QFileInfo>
#include <QDebug>
#include <QTemporaryFile>
#include <QDir>

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
#include "qimg2dcm.h"

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

bool DicomProcessor::save(QString filePath, QList<QImage> newImages)
{
    if(newImages.length()<1)
        return false;

    Qimg2dcm q2dcm;
    DcmDataset dsCopy = *(this->dicomFile->getDataset());
    E_TransferSyntax writeXfer = EXS_LittleEndianImplicit;
    unsigned long length;
    char* pixData = NULL;
    const Uint8* pixDataC;

    q2dcm.insertImage(dsCopy, writeXfer, newImages.at(0));
    dsCopy.findAndGetUint8Array(DCM_PixelData, pixDataC, &length);
    pixData= (char*) pixDataC;

    for (int i = 1; i < newImages.size(); ++i) {
        DcmDataset dsSingleFrameCopy = *(this->dicomFile->getDataset());
        q2dcm.insertImage(dsSingleFrameCopy, writeXfer, newImages.at(i));
        unsigned long f_length;

        dsSingleFrameCopy.findAndGetUint8Array(DCM_PixelData, pixDataC, &f_length);
        pixData = (char*) realloc(pixData, (length+f_length)*sizeof(Uint8));
        memcpy(pixData+(length*sizeof(Uint8)), pixDataC, (f_length*sizeof(Uint8)));
        length+=f_length;
    }

    dsCopy.putAndInsertUint8Array(DCM_PixelData, OFreinterpret_cast(Uint8*, pixData), length);

    dsCopy.saveFile(filePath.toLocal8Bit(), writeXfer);
    return true; //failure is not an option
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
