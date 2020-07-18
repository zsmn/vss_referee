#ifndef VSSREPLACER_H
#define VSSREPLACER_H

#include <src/entity/entity.h>
#include <include/vssclient/vssclient.h>
#include <include/vssref_placement.pb.h>

class VSSReplacer : public Entity
{
public:
    VSSReplacer(const std::string& refereeAddress, int replacerPort);
    QString name();

private:
    void initialization();
    void loop();
    void finalization();

    // VSS Client to receive replacement data
    VSSClient *_vssClient;
    std::string _refereeAddress;
    int _replacerPort;
};

#endif // VSSREPLACER_H
