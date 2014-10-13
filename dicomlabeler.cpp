#include <QFileInfo>
#include <QImage>
#include <QString>
#include <QStringList>
#include <QWebFrame>

#include "dicomlabeler.h"
#include "templaterenderer.h"

DicomLabeler::DicomLabeler()
{
    label_x=0;
    label_y=0;

    mode = DicomLabelerMode_default;

    this->tmplRenderer.setDcmProcessor(&this->dcmProcessor);

    connect(&tmplRenderer, SIGNAL(pageRendered(QImage*)),
            this, SLOT(templateRendered(QImage*)));
}

DicomLabeler::~DicomLabeler() {
}

void DicomLabeler::startProcessing(DicomLabelerMode mode)
{
    this->mode = mode;
    this->setTemplate(this->templateFile);
}


void DicomLabeler::setTemplate(const QString templateFile) {
    tmplRenderer.getPage()->mainFrame()->load(QUrl::fromUserInput( templateFile ));
}

QString DicomLabeler::getInputFile() const
{
    return inputFile;
}

void DicomLabeler::setInputFile(const QString &value)
{
    inputFile = value;
    dcmProcessor.load(this->inputFile);
}

QString DicomLabeler::getOutputFile() const
{
    return outputFile;
}

void DicomLabeler::setOutputFile(const QString &value)
{
    outputFile = value;
}

QString DicomLabeler::getTemplateFile() const
{
    return templateFile;
}

void DicomLabeler::setTemplateFile(const QString &value)
{
    QFileInfo fi(value);
    templateFile = fi.absoluteFilePath();
}

int DicomLabeler::getSelectedFrame()
{
    return (selectedFrame>=0&&selectedFrame<(int)this->dcmProcessor.frameCount())?selectedFrame:this->dcmProcessor.getRepresentativeFrame();
}

void DicomLabeler::setSelectedFrame(int value)
{
    selectedFrame = value;
}


long DicomLabeler::getLabel_y() const
{
    return label_y;
}

void DicomLabeler::setLabel_y(long value)
{
    label_y = value;
}

long DicomLabeler::getLabel_x() const
{
    return label_x;
}

void DicomLabeler::setLabel_x(long value)
{
    label_x = value;
}

TemplateRenderer *DicomLabeler::getTemplateRenderer()
{
    return &tmplRenderer;
}

void DicomLabeler::templateRendered(QImage *templateImage)
{
    this->labelImage = templateImage->convertToFormat(QImage::Format_ARGB32);

    switch(mode) {
    case DicomLabelerMode_default:
        {
            this->dcmProcessor.save(this->outputFile, this->labelImage, this->label_x, this->label_y);
        }
        break;
    case DicomLabelerMode_image_only:
        {
            // TODO
        }
        break;
    case DicomLabelerMode_template_only:
        this->saveImage(this->labelImage, this->outputFile);
        break;
    }

    QApplication::exit();
}

void DicomLabeler::saveImage(QImage& image, QString filepath) {
    image.save(filepath);
}
