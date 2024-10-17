#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QCommandLineParser>
#include <QDebug>
#include <QQmlContext>
#include "prompt_dialog.h"
#include <iostream>


int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    QCommandLineParser parser;
    parser.setApplicationDescription("DeepMake Prompt App");
    parser.addHelpOption();
    parser.addVersionOption();

    // Add the option for -p followed by a string
    QCommandLineOption optionP("p", "Specify a prompt string after -p", "string");
    parser.addOption(optionP);
    parser.process(app);

    QString defaultPrompt;
    if (parser.isSet(optionP)) {
        defaultPrompt = parser.value(optionP);
    }

    PromptDialog promptDialog;
    if (!defaultPrompt.isEmpty())
        promptDialog.setPromptText(defaultPrompt);
    QQmlApplicationEngine engine;

    QQmlContext *context = engine.rootContext();
    context->setContextProperty("PromptDialog", &promptDialog);

    const QUrl url(u"qrc:/Prompt/Main.qml"_qs);

    QObject::connect(&engine, &QQmlApplicationEngine::objectCreationFailed,
        &app, []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);
    engine.load(url);

    int ret = app.exec();
    std::cout << promptDialog.getPromptText().toStdString();

    return ret;
}
