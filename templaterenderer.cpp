#include "templaterenderer.h"

#include <QApplication>
#include <QtWebKit>
#include <QtGui>

#include <QTimer>
#include <QByteArray>
#include <QNetworkRequest>
#include <QNetworkProxy>

#include <QRegExp>

#ifdef STATIC_PLUGINS
Q_IMPORT_PLUGIN(qjpeg)
Q_IMPORT_PLUGIN(qgif)
Q_IMPORT_PLUGIN(qtiff)
Q_IMPORT_PLUGIN(qmng)
Q_IMPORT_PLUGIN(qico)
#endif

#include <dicomprocessor.h>

QString
TemplatePage::chooseFile(QWebFrame* /*frame*/, const QString& /*suggestedFile*/) {
    return QString::null;
}

bool
TemplatePage::javaScriptConfirm(QWebFrame* /*frame*/, const QString& /*msg*/) {
    return true;
}

bool
TemplatePage::javaScriptPrompt(QWebFrame* /*frame*/,
                               const QString& /*msg*/,
                               const QString& /*defaultValue*/,
                               QString* /*result*/) {
    return true;
}

void
TemplatePage::javaScriptConsoleMessage(const QString& /*message*/,
                                       int /*lineNumber*/,
                                       const QString& /*sourceID*/) {
    // noop
}

void
TemplatePage::javaScriptAlert(QWebFrame* /*frame*/, const QString& /*msg*/) {
    // noop
}

void
TemplatePage::setAttribute(QWebSettings::WebAttribute option, bool value) {
    settings()->setAttribute(option, value);
}


TemplateRenderer::TemplateRenderer() {
    renderedImage = NULL;
    mSawInitialLayout = false;
    mSawDocumentComplete = false;
    dcmProcessor = NULL;


#if CUTYCAPT_SCRIPT
    // javaScriptWindowObjectCleared does not get called on the
    // initial load unless some JavaScript has been executed.
    mPage.mainFrame()->evaluateJavaScript(QString(""));

    connect(mPage.mainFrame(),
            SIGNAL(javaScriptWindowObjectCleared()),
            &main,
            SLOT(JavaScriptWindowObjectCleared()));
#endif

    connect(mPage.networkAccessManager(),
            SIGNAL(sslErrors(QNetworkReply*, QList<QSslError>)),
            this,
            SLOT(handleSslErrors(QNetworkReply*, QList<QSslError>)));

    connect(&mPage,
            SIGNAL(loadFinished(bool)),
            this,
            SLOT(DocumentComplete(bool)));

    connect(mPage.mainFrame(),
            SIGNAL(initialLayoutCompleted()),
            this,
            SLOT(InitialLayoutCompleted()));
}

TemplateRenderer::~TemplateRenderer()
{
    if(this->renderedImage) delete this->renderedImage;
}

void TemplateRenderer::setTemplatePath(QString templateFilePath)
{
    mPage.mainFrame()->load(QUrl::fromUserInput( templateFilePath ));
}

TemplatePage *TemplateRenderer::getPage()
{
    return &mPage;
}

void
TemplateRenderer::InitialLayoutCompleted() {
    mSawInitialLayout = true;

    if (mSawInitialLayout && mSawDocumentComplete)
        finishRender();
}

void
TemplateRenderer::DocumentComplete(bool /*ok*/) {
    mSawDocumentComplete = true;

    if (mSawInitialLayout && mSawDocumentComplete)
        finishRender();
}

void
TemplateRenderer::JavaScriptWindowObjectCleared() {
}

void
TemplateRenderer::finishRender() {
    saveSnapshot();
}

void
TemplateRenderer::handleSslErrors(QNetworkReply* reply, QList<QSslError> /*errors*/) {
    reply->ignoreSslErrors();
}

void TemplateRenderer::Timeout()
{
    saveSnapshot();
}

void
TemplateRenderer::saveSnapshot() {
    QWebFrame *mainFrame = mPage.mainFrame();
    QPainter painter;


    mPage.blockSignals(true);
    mainFrame->blockSignals(true);
    QString htmlCode = mainFrame->toHtml();
    mainFrame->setHtml(processHtmlCode(htmlCode));
    mainFrame->blockSignals(false);
    mPage.blockSignals(false);

    // TODO: sometimes contents/viewport can have size 0x0
    // in which case saving them will fail. This is likely
    // the result of the method being called too early. So
    // far I've been unable to find a workaround, except
    // using --delay with some substantial wait time. I've
    // tried to resize multiple time, make a fake render,
    // check for other events... This is primarily a problem
    // under my Ubuntu virtual machine.

    mPage.setViewportSize( mainFrame->contentsSize() );


    this->renderedImage = new QImage(mPage.viewportSize(), QImage::Format_ARGB32);
    painter.begin(this->renderedImage);
#if QT_VERSION >= 0x050000
    //if (mSmooth)
    {
        painter.setRenderHint(QPainter::SmoothPixmapTransform);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setRenderHint(QPainter::TextAntialiasing);
        painter.setRenderHint(QPainter::HighQualityAntialiasing);
    }
#endif
    mainFrame->render(&painter);
    painter.end();
    // TODO: add quality

    emit pageRendered(this->renderedImage);
}

QString TemplateRenderer::processHtmlCode(QString htmlCode)
{
    QRegExp rxLTags("#\\((\\w+)\\)");

    while ((rxLTags.indexIn(htmlCode, 0)) != -1) {
        QString name = rxLTags.cap(1);
        htmlCode.replace(rxLTags.cap(0), getDicomValue(name));
    }

    return htmlCode;
}

QString TemplateRenderer::getDicomValue(QString name)
{
    if(dcmProcessor) {
        return dcmProcessor->getMetadata(name);
    }

    return QString("!" + name);
}

void TemplateRenderer::setDcmProcessor(DicomProcessor *value)
{
    dcmProcessor = value;
}


