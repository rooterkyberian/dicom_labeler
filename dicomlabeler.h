#ifndef DICOMLABELER_H
#define DICOMLABELER_H

#include <QString>
#include <QImage>

#include "templaterenderer.h"
#include "dicomprocessor.h"

enum DicomLabelerMode {
    DicomLabelerMode_default,
    DicomLabelerMode_template_only,
    DicomLabelerMode_image_only
};

class DicomLabeler : public QObject {
    Q_OBJECT
public:
    DicomLabeler();
    ~DicomLabeler();
    void startProcessing(QString templateFile, QString inputFile, QString outputFile, DicomLabelerMode mode);

    TemplateRenderer *getTemplateRenderer();

    long getLabel_x() const;
    void setLabel_x(long value);

    long getLabel_y() const;
    void setLabel_y(long value);

private slots:
    void templateRendered(QImage *templateImage);


private:
    void saveImage(QImage& image, QString filepath);
    QImage overlayImage(const QImage& baseImage, const QImage& overlayImage, int x, int y);
    void setTemplate(const QString templateFile);

    QString templateFile, outputFile, inputFile;
    DicomLabelerMode mode;

    DicomProcessor dcmProcessor;

    TemplateRenderer tmplRenderer;
    long int label_x, label_y;
    QImage labelImage;
};

#endif // DICOMLABELER_H
