#ifndef VSSREFEREE_H
#define VSSREFEREE_H

#include <src/entity/vssvisionclient/vssvisionclient.h>
#include <include/vssref_command.pb.h>
#include <include/vssref_placement.pb.h>

#include <QtCore>
#include <QUdpSocket>
#include <include/timer.h>

class VSSReferee : public Entity
{
public:
    VSSReferee(VSSVisionClient *visionClient, const QString& refereeAddress, int refereePort);
    ~VSSReferee();
    QString name();

    // Network
    bool connect(const QString& refereeAddress, int refereePort);
    void disconnect();
    bool isConnected() const;

private:
    void initialization();
    void loop();
    void finalization();

    // Socket to send foul data
    QUdpSocket _socket;
    QString _refereeAddress;
    int _refereePort;
    VSSRef::ref_to_team::VSSRef_Command _refereeCommand;
    void sendPacket(VSSRef::ref_to_team::VSSRef_Command command);

    // VSSVisionClient to receive data from FIRASim
    VSSVisionClient *_visionClient;
};

#endif // VSSREFEREE_H
