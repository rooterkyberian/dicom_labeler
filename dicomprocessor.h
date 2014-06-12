#ifndef DICOMPROCESSOR_H
#define DICOMPROCESSOR_H

#include<QString>
#include<QList>
#include<QImage>


class DicomImage;
class DcmFileFormat;
class DcmTagKey; //dcmtk
class DicomProcessor
{
public:
    DicomProcessor();
    ~DicomProcessor();

    QString getMetadata(QString id);
    bool setImages(QList<QImage>);
    void load(QString filePath);
    bool save(QString filePath);

    unsigned long frameCount() const;
    QImage frame(int frame) const;

private:
    DicomImage *dicomImage;
    DcmFileFormat *dicomFile;
    void loadDicomImage(DicomImage* dicomImage);
    static DcmTagKey parseTagKey(const char *tagName);
};

#endif // DICOMPROCESSOR_H
