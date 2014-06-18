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

void DicomLabeler::startProcessing(QString templateFile, QString inputFile, QString outputFile, DicomLabelerMode mode)
{
    this->templateFile = templateFile;
    this->inputFile = inputFile;
    this->outputFile = outputFile;
    this->mode = mode;

    switch(mode) {
    case DicomLabelerMode_default:
    case DicomLabelerMode_image_only:
        dcmProcessor.load(this->inputFile);
        // read DICOM metadata
        // fill template renderer
        break;
    case DicomLabelerMode_template_only:
        break;
    }

    this->setTemplate(this->templateFile);
}


void DicomLabeler::setTemplate(const QString templateFile) {
    tmplRenderer.getPage()->mainFrame()->load(QUrl::fromUserInput( templateFile ));
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
            QList<QImage> overlayedImages;
            for(unsigned int i=0; i<this->dcmProcessor.frameCount(); i++)
            {
                QImage merged = overlayImage(this->dcmProcessor.frame(i),
                                             this->labelImage, this->label_x, this->label_y);
                overlayedImages.append(merged);
            }
            this->dcmProcessor.save(this->outputFile, overlayedImages);
        }
        break;
    case DicomLabelerMode_image_only:
        {
            QImage merged = overlayImage(this->dcmProcessor.frame(this->getSelectedFrame()),
                                         this->labelImage, this->label_x, this->label_y);
            this->saveImage(merged, this->outputFile);
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

QImage DicomLabeler::overlayImage(const QImage& baseImage, const QImage& overlayImage, int x, int y)
{
    QImage imageWithOverlay = QImage(baseImage.size(), QImage::Format_ARGB32);
    QPainter painter(&imageWithOverlay);

    painter.setCompositionMode(QPainter::CompositionMode_Source);
    painter.fillRect(imageWithOverlay.rect(), Qt::transparent);

    painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
    painter.drawImage(0, 0, baseImage);

    if(x<0) x+=baseImage.size().width()-overlayImage.size().width();
    if(y<0) y+=baseImage.size().height()-overlayImage.size().height();

    painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
    painter.drawImage(x, y, overlayImage);

    painter.end();

    return imageWithOverlay;
}
