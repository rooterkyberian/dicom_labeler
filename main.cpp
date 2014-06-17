#include <QCoreApplication>
#include <QApplication>
#include <QtWebKit>
#include <QtGui>

#if QT_VERSION < 0x050000
#include <QPrinter>
#endif

#include <QTimer>
#include <QByteArray>
#include <QNetworkRequest>
#include <QNetworkProxy>
#include <QCommandLineParser>

#include <dcmtk/dcmjpeg/djdecode.h>
#include <dcmtk/dcmjpeg/dipijpeg.h>
#include <dcmtk/dcmjpls/djdecode.h>
#include <dcmtk/dcmimage/dipitiff.h>
#include <dcmtk/dcmimage/dipipng.h>
#include <dcmtk/dcmdata/dcrledrg.h>

#include "templaterenderer.h"
#include "dicomlabeler.h"
#include "dicom_labeler_info.h"

void showErrorHelpAndExit(QCommandLineParser &parser, QString message) {
    qDebug() << message;
    parser.showHelp(1);
}

void parseCommandLine(QApplication &app, DicomLabeler &dicomLabeler)
{
    QCommandLineParser parser;
    parser.setApplicationDescription(
                APP_NAME" "APP_VERSION"\n"
                "Usage:\n"
                "\t-x=10 -y=100 template.html output.dcm input.dcm\n"
                "\t-I -x=-20 -y=-20 template.html output.png input.dcm\n"
                "\t-T template.html output.png"
                );
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption ouputTemplateOnly(QStringList() << "T" << "template_only", QApplication::translate("main", "Only output processed template"));
    parser.addOption(ouputTemplateOnly);
    QCommandLineOption ouputImageOnly(QStringList() << "I" << "image_only", QApplication::translate("main", "Only output modified image"));
    parser.addOption(ouputImageOnly);

    QCommandLineOption labelPositionXOption(QStringList() << "x" << "label-position-x",
                                            QCoreApplication::translate("main", "Overlay label coordinate <x>."),
                                            QCoreApplication::translate("main", "x"));
    labelPositionXOption.setDefaultValue("0");
    parser.addOption(labelPositionXOption);

    QCommandLineOption labelPositionYOption(QStringList() << "y" << "label-position-y",
                                            QCoreApplication::translate("main", "Overlay label coordinate <y>."),
                                            QCoreApplication::translate("main", "y"));
    labelPositionYOption.setDefaultValue("0");
    parser.addOption(labelPositionYOption);

    QCommandLineOption labelWidthOption(QStringList() << "W" << "label-min-width",
                                        QCoreApplication::translate("main", "Label minimal <width>."),
                                        QCoreApplication::translate("main", "width"));
    labelWidthOption.setDefaultValue("0");
    parser.addOption(labelWidthOption);

    QCommandLineOption labelHeightOption(QStringList() << "H" << "label-min-height",
                                         QCoreApplication::translate("main", "Label minimal <height>."),
                                         QCoreApplication::translate("main", "height"));
    labelHeightOption.setDefaultValue("0");
    parser.addOption(labelHeightOption);

    QCommandLineOption labelScaleOption(QStringList() << "s" << "label-scale",
                                        QCoreApplication::translate("main", "Label scaling factor <scale>."),
                                        QCoreApplication::translate("main", "scale"));
    labelScaleOption.setDefaultValue("1.0");
    parser.addOption(labelScaleOption);

    parser.addPositionalArgument("template", QApplication::translate("main", "Template file."));
    parser.addPositionalArgument("output", QApplication::translate("main", "Output file."));
    parser.addPositionalArgument("[dicom_input]", QApplication::translate("main", "Input DICOM file."));

    parser.process(app);

    const QStringList args = parser.positionalArguments();

    if(parser.isSet(ouputImageOnly) && parser.isSet(ouputTemplateOnly))
    {
        showErrorHelpAndExit(parser, QString("Template & Image only modes exclude each other"));
    }

    TemplatePage *page = dicomLabeler.getTemplateRenderer()->getPage();

    if(parser.value(labelWidthOption).toInt()>0 && parser.value(labelHeightOption).toInt()>0)
        page->setViewportSize( QSize(parser.value(labelWidthOption).toInt(), parser.value(labelHeightOption).toInt()) );

    if (parser.isSet(labelScaleOption)) {
        page->mainFrame()->setZoomFactor(parser.value(labelScaleOption).toFloat());
    }

    switch(args.count()) {
    case 3:
        dicomLabeler.setLabel_x(parser.value(labelPositionXOption).toInt());
        dicomLabeler.setLabel_y(parser.value(labelPositionYOption).toInt());

        if(parser.isSet(ouputImageOnly))
        {
            // read dicom
            // generate template
            // merge
            // write dicom
            dicomLabeler.startProcessing(args.at(0), args.at(2), args.at(1), DicomLabelerMode_image_only);
        } else {
            // read dicom
            // generate template
            // merge
            // write image
            dicomLabeler.startProcessing(args.at(0), args.at(2), args.at(1), DicomLabelerMode_default);
        }
        break;
    case 2:
        if(parser.isSet(ouputTemplateOnly))
        {
            // generate template
            // write to file
            dicomLabeler.startProcessing(args.at(0), "", args.at(1), DicomLabelerMode_template_only);
        } else {
            showErrorHelpAndExit(
                        parser,
                        QString("wrong number of positional arguments for this mode (%1)").arg(
                            QString::number(args.count())
                            )
                        );
        }
        break;
    default:
        showErrorHelpAndExit(
                    parser,
                    QString("wrong number of positional arguments (%1)").arg(
                        QString::number(args.count())
                        )
                    );
        break;
    }
}

int main(int argc, char *argv[])
{
    // register RLE decompression codec
    DcmRLEDecoderRegistration::registerCodecs();
    // register JPEG decompression codecs
    DJDecoderRegistration::registerCodecs(EDC_guess);
    // register JPEG-LS decompression codecs
    DJLSDecoderRegistration::registerCodecs();


    QApplication app(argc, argv, true);
    QApplication::setApplicationName(APP_NAME);
    QApplication::setApplicationVersion(APP_VERSION);

    DicomLabeler dicomLabeler;

    parseCommandLine(app, dicomLabeler);

/*
    // deregister RLE decompression codec
    DcmRLEDecoderRegistration::cleanup();
#ifdef BUILD_DCM2PNM_AS_DCMJ2PNM
    // deregister JPEG decompression codecs
    DJDecoderRegistration::cleanup();
#endif
#ifdef BUILD_DCM2PNM_AS_DCML2PNM
    // deregister JPEG-LS decompression codecs
    DJLSDecoderRegistration::cleanup();
#endif
*/


    return app.exec();
}
