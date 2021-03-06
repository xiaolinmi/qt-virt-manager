#include "block_disk.h"

Block_Disk::Block_Disk(
        QWidget         *parent,
        virConnectPtr   *connPtrPtr) :
    _Disk(parent, connPtrPtr)
{
    browse = new QPushButton(tr("Block device:"), this);
    path = new QLineEdit(this);
    path->setPlaceholderText("/dev/sda");

    baseLayout->addWidget(browse, 0, 0);
    baseLayout->addWidget(path, 0, 1);

    secLabels->setVisible(true);

    connect(browse, SIGNAL(clicked()),
            this, SLOT(setDevPath()));
    // dataChanged connections
    connect(path, SIGNAL(textChanged(QString)),
            this, SLOT(stateChanged()));
}

/* public slots */
QDomDocument Block_Disk::getDataDocument() const
{
    QDomDocument doc;
    QDomElement _source, _target, _device, _devDesc,
            _encrypt, _secret;
    _device = doc.createElement("device");
    _devDesc = doc.createElement("disk");

    _source = doc.createElement("source");
    _source.setAttribute("dev", path->text());
    if ( startupPolicy->isUsed() )
        _source.setAttribute("startupPolicy",
                             startupPolicy->getStartupPolicy());
    _devDesc.appendChild(_source);

    _target = doc.createElement("target");
    TargetAttrs l = target->getTargetAttrs();
    foreach (QString key, l.keys()) {
        if ( !key.isEmpty() )
        _target.setAttribute(key, l.value(key));
    };
    _devDesc.appendChild(_target);

    if ( secLabels->isUsed() ) {
        QDomNodeList _l = secLabels->getDataDocument()
                .firstChildElement("data")
                .childNodes();
        int j = 0;
        int count = _l.length();
        for (int i=0; i<count;i++) {
            //qDebug()<<_l.item(j).nodeName()<<i;
            if (!_l.item(j).isNull()) _devDesc.appendChild(_l.item(j));
            else ++j;
        };
    };

    if ( encrypt->isUsed() ) {
        _encrypt = doc.createElement("encryption");
        _encrypt.setAttribute("format", "qcow");
        _secret = doc.createElement("secret");
        _secret.setAttribute("type", "passphrase");
        _secret.setAttribute("uuid", encrypt->getSecretUUID());
        _encrypt.appendChild(_secret);
        _devDesc.appendChild(_encrypt);
    };

    if ( readOnly->state() ) {
        QDomElement _readOnly = doc.createElement("readonly");
        _devDesc.appendChild(_readOnly);
    };

    if ( addr->use->isChecked() &&
         addr->getCurrentAddrWidget()->isEnabled() ) {
        QDomElement _addr = doc.createElement("address");
        AttrList _l = addr->getAttrList();
        foreach (QString _attr, _l.keys()) {
            if ( !_attr.isEmpty() )
                _addr.setAttribute( _attr, _l.value(_attr) );
        };
        _devDesc.appendChild(_addr);
    };

    _devDesc.setAttribute("type", "block");
    _devDesc.setAttribute("device", devType->getDeviceType());
    _device.appendChild(_devDesc);
    doc.appendChild(_device);
    return doc;
}
void Block_Disk::setDataDescription(const QString &_xmlDesc)
{
    //qDebug()<<_xmlDesc;
    QDomDocument doc;
    doc.setContent(_xmlDesc);
    QDomElement _device, _source, _target, _encrypt,
            _secret, _readOnly, _secLabel, _addr, _driver;
    _device = doc.firstChildElement("device")
            .firstChildElement("disk");
    _source = _device.firstChildElement("source");
    _target = _device.firstChildElement("target");
    _secLabel = _device.firstChildElement("seclabel");
    _readOnly = _device.firstChildElement("readonly");
    _addr = _device.firstChildElement("address");
    _driver = _device.firstChildElement("driver");
    QString _attr;
    int idx;
    _attr = _device.attribute("device");
    devType->setDeviceType(_attr);
    _attr = _source.attribute("dev");
    path->setText(_attr);
    if ( _source.hasAttribute("startupPolicy") ) {
        startupPolicy->setUsage(true);
        _attr = _source.attribute("startupPolicy");
        idx = startupPolicy->findPolicyIndex(_attr);
        startupPolicy->setPolicyIndex( (idx<0)? 0:idx );
    } else
        startupPolicy->setUsage(false);
    _attr = _target.attribute("dev");
    target->devName->setText(_attr);
    _attr = _target.attribute("bus");
    idx = target->bus->findText(_attr, Qt::MatchContains);
    target->bus->setCurrentIndex( (idx<0)? 0:idx );
    if ( _target.hasAttribute("tray") ) {
        target->trayLabel->setChecked(true);
        _attr = _target.attribute("tray");
        idx = target->tray->findText(_attr, Qt::MatchContains);
        target->tray->setCurrentIndex( (idx<0)? 0:idx );
    } else
        target->trayLabel->setChecked(false);
    if ( _target.hasAttribute("removable") ) {
        target->removableLabel->setChecked(true);
        _attr = _target.attribute("removable");
        idx = target->removable->findText(_attr, Qt::MatchContains);
        target->removable->setCurrentIndex( (idx<0)? 0:idx );
    } else
        target->removableLabel->setChecked(false);
    secLabels->setUsage( !_secLabel.isNull() );
    if ( !_secLabel.isNull() ) {
        QDomDocument _doc;
        _doc.setContent(QString());
        _doc.appendChild(_device);
        _device.setTagName("domain");
        QString _xmlDesc = _doc.toString();
        secLabels->readXMLDesciption(_xmlDesc);
    };
    encrypt->setUsage( !_encrypt.isNull() );
    if ( !_encrypt.isNull() ) {
        _secret = _encrypt.firstChildElement("secret");
        encrypt->setSecretUUID(_secret.attribute("uuid"));
    };
    readOnly->readOnly->setChecked( !_readOnly.isNull() );
    addr->use->setChecked( !_addr.isNull() );
    if ( !_addr.isNull() ) {
        _attr = _addr.attribute("type");
        idx = addr->type->findData(
                    _attr,
                    Qt::UserRole,
                    Qt::MatchContains);
        addr->type->setCurrentIndex( (idx<0)? 0:idx );
        if ( _attr.compare("pci")==0 ) {
            PciAddr *wdg = static_cast<PciAddr*>(addr->getCurrentAddrWidget());
            if ( wdg!=Q_NULLPTR ) {
                wdg->domain->setText( _addr.attribute("domain") );
                wdg->bus->setText( _addr.attribute("bus") );
                wdg->slot->setText( _addr.attribute("slot") );
                wdg->function->setValue( _addr.attribute("function")
                                         .split("x").last().toInt() );
                if ( _addr.hasAttribute("multifunction") ) {
                    wdg->multifunction->setEnabled(true);
                    wdg->multifunction->setChecked(
                                _addr.attribute("multifunction").compare("on")==0 );
                };
            };
        } else if ( _attr.compare("drive")==0 ) {
            DriveAddr *wdg = static_cast<DriveAddr*>( addr->getCurrentAddrWidget() );
            if ( wdg!=Q_NULLPTR ) {
                wdg->controller->setText( _addr.attribute("controller") );
                wdg->bus->setText( _addr.attribute("bus") );
                wdg->target->setText( _addr.attribute("target") );
                wdg->unit->setText( _addr.attribute("unit") );
            };
        };
    };
}

/* private slots */
void Block_Disk::setDevPath()
{
    QString filePath = QFileDialog::getOpenFileName(
                this,
                tr("Block Device Path"),
                "/dev");
    if ( !filePath.isEmpty() ) path->setText(filePath);
}
