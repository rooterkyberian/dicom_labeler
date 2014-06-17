#ifndef QIMG2DCM_H
#define QIMG2DCM_H

// QT
#include <QImage>
#include <QString>

// DCMTK includes
#include <dcmtk/dcmdata/dcfilefo.h>
#include <dcmtk/ofstd/ofbmanip.h>
#include <dcmtk/dcmdata/libi2d/i2d.h>

class Qimg2dcm : public Image2Dcm
{
public:
    Qimg2dcm();
    void insertImage(DcmDataset &outputDataset, E_TransferSyntax &writeXfer, const QImage &image);
};

#endif // QIMG2DCM_H
