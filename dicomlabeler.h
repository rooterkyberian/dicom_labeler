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
    void startProcessing(DicomLabelerMode mode);

    TemplateRenderer *getTemplateRenderer();

    long getLabel_x() const;
    void setLabel_x(long value);

    long getLabel_y() const;
    void setLabel_y(long value);

    int getSelectedFrame();
    void setSelectedFrame(int value);

    QString getTemplateFile() const;
    void setTemplateFile(const QString &value);

    QString getOutputFile() const;
    void setOutputFile(const QString &value);

    QString getInputFile() const;
    void setInputFile(const QString &value);

private slots:
    void templateRendered(QImage *templateImage);

private:
    void saveImage(QImage& image, QString filepath);
    void setTemplate(const QString templateFile);

    QString templateFile, outputFile, inputFile;
    DicomLabelerMode mode;

    DicomProcessor dcmProcessor;

    TemplateRenderer tmplRenderer;
    long int label_x, label_y;
    QImage labelImage;

    int selectedFrame;
};

#endif // DICOMLABELER_H
