#include "qimg2dcm.h"

// QT
#include <QImage>
#include <QString>
#include <QTemporaryFile>

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
#include <dcmtk/dcmdata/libi2d/i2d.h>
#include <dcmtk/dcmdata/libi2d/i2dbmps.h>
#include "dcmtk/dcmdata/libi2d/i2dplsc.h"
#include <dcmtk/dcmdata/dcdeftag.h>

Qimg2dcm::Qimg2dcm() : Image2Dcm()
{
}

void Qimg2dcm::insertImage(DcmDataset &outputDataset, E_TransferSyntax &writeXfer, const QImage &image)
{
    QTemporaryFile tmpImageFile("XXXXXX.bmp");
    tmpImageFile.open();
    tmpImageFile.close();

    image.save(tmpImageFile.fileName());
    I2DImgSource *imageSource = new I2DBmpSource();
    imageSource->setImageFile(OFString(tmpImageFile.fileName().toLocal8Bit()));

    OFCondition cond;
    cleanupTemplate(&outputDataset);
    cond = generateUIDs(&outputDataset);
    // return cond if failed
    cond = readAndInsertPixelData(imageSource, &outputDataset, writeXfer);

    I2DOutputPlug *outPlug = new I2DOutputPlugSC();

    cond = outPlug->convert(outputDataset);

    delete outPlug;
    delete imageSource;

    // return cond
}
