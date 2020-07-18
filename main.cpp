#include <QCoreApplication>

#include <src/entity/vssvisionclient/vssvisionclient.h>
#include <src/entity/vssreferee/vssreferee.h>
#include <src/entity/vssreplacer/vssreplacer.h>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    // Defines
    QString refereeAddress = "224.5.23.2";
    int visionPort = 10002;
    int refereePort = 10003;
    int replacerPort = 10004;

    // Create modules
    VSSVisionClient vssVisionClient(refereeAddress, visionPort);
    VSSReferee vssReferee(&vssVisionClient, refereeAddress, refereePort);
    VSSReplacer vssReplacer(refereeAddress.toStdString(), replacerPort);

    // Start all
    vssVisionClient.start();
    vssReferee.start();
    vssReplacer.start();

    // Run
    bool exec = a.exec();

    // Stop modules
    vssVisionClient.terminate();
    vssReferee.terminate();
    vssReplacer.terminate();

    // Wait for modules sync
    vssVisionClient.wait();
    vssReferee.wait();
    vssReplacer.wait();

    return exec;
}
