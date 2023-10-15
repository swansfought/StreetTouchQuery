#include "record.h"

Record::Record():buttonid(0),title(""),subtitle(""),context("")
{
}

Record::~Record()
{
}

int Record::getButtonid() const
{
    return buttonid;
}

void Record::setButtonid(int newButtonid)
{
    buttonid = newButtonid;
}

QString Record::getTitle() const
{
    return title;
}

void Record::setTitle(const QString &newTitle)
{
    title = newTitle;
}

QString Record::getSubtitle() const
{
    return subtitle;
}

void Record::setSubtitle(const QString &newSubtitle)
{
    subtitle = newSubtitle;
}

QString Record::getContext() const
{
    return context;
}

void Record::setContext(const QString &newContext)
{
    context = newContext;
}

QByteArray Record::getImage() const
{
    return image;
}

void Record::setImage(const QByteArray &newImage)
{
    image = newImage;
}

QDateTime Record::getUpdatetime() const
{
    return updatetime;
}

void Record::setUpdatetime(const QDateTime &newUpdatetime)
{
    updatetime = newUpdatetime;
}

int Record::getShowmode() const
{
    return showmode;
}

void Record::setShowmode(int newShowmode)
{
    showmode = newShowmode;
}

