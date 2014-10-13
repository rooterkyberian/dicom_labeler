#ifndef DICOMPROCESSOR_H
#define DICOMPROCESSOR_H

#include<QString>
#include<QList>
#include<QImage>


class DicomImage;
class DcmFileFormat;
class DcmTagKey;
class DcmDataset;
class DicomProcessor
{
public:
    DicomProcessor();
    ~DicomProcessor();

    QString getMetadata(QString id);
    void load(QString filePath);
    bool save(QString filePath);

    unsigned long frameCount() const;
    QImage frame(int frame) const;
    int getRepresentativeFrame();

    void overlay(QImage label, int x, int y);

private:
    DicomImage *dicomImage;
    DcmFileFormat *dicomFile;
    void loadDicomImage(DicomImage* dicomImage);
    static DcmTagKey parseTagKey(const char *tagName);
};

#endif // DICOMPROCESSOR_H
