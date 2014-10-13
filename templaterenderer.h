#ifndef HTMLRENDERER_H
#define HTMLRENDERER_H


#include <QtWebKit>
#include <QNetworkReply>
#include <QSslError>

#if QT_VERSION >= 0x050000
#include <QtWebKitWidgets>
#endif

class TemplateRenderer;
class TemplatePage : public QWebPage {
    Q_OBJECT

public:
    TemplatePage() : QWebPage() {
            setAttribute(QWebSettings::AutoLoadImages, true);
            setAttribute(QWebSettings::JavascriptEnabled, true);
            setAttribute(QWebSettings::LinksIncludedInFocusChain, true);
            setAttribute(QWebSettings::PrintElementBackgrounds, true);


            // The documentation does not say, but it seems the mainFrame
            // will never change, so we can set this here. Otherwise we'd
            // have to set this in snapshot and trigger an update, which
            // is not currently possible (Qt 4.4.0) as far as I can tell.
            mainFrame()->setScrollBarPolicy(Qt::Horizontal, Qt::ScrollBarAlwaysOff);
            mainFrame()->setScrollBarPolicy(Qt::Vertical, Qt::ScrollBarAlwaysOff);
        }
    void setAttribute(QWebSettings::WebAttribute option, bool value);

protected:
    QString chooseFile(QWebFrame *frame, const QString& suggestedFile);
    void javaScriptConsoleMessage(const QString& message, int lineNumber, const QString& sourceID);
    bool javaScriptPrompt(QWebFrame* frame, const QString& msg, const QString& defaultValue, QString* result);
    void javaScriptAlert(QWebFrame* frame, const QString& msg);
    bool javaScriptConfirm(QWebFrame* frame, const QString& msg);
};

class DicomProcessor;
class TemplateRenderer : public QObject {
    Q_OBJECT

public:
    TemplateRenderer();
    ~TemplateRenderer();
    void setTemplatePath(QString templateFilePath);
    TemplatePage *getPage();

    void setDcmProcessor(DicomProcessor *dcmProcessor);

signals:
    void pageRendered(QImage *image);

private slots:
    void DocumentComplete(bool ok);
    void InitialLayoutCompleted();
    void JavaScriptWindowObjectCleared();
    void handleSslErrors(QNetworkReply* reply, QList<QSslError> errors);
    void Timeout();

private:
    void finishRender();
    void saveSnapshot();
    QString processHtmlCode(QString htmlCode);
    QString getDicomValue(QString Name);
    bool mSawInitialLayout;
    bool mSawDocumentComplete;
    QImage *renderedImage;
    DicomProcessor *dcmProcessor;


protected:
    TemplatePage    mPage;

};


#endif // HTMLRENDERER_H
