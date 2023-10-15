#ifndef RECORD_H
#define RECORD_H

#include <QString>
#include <QByteArray>
#include <QDateTime>

class Record
{
public:
    Record();
    ~Record();

    int getButtonid() const;
    void setButtonid(int newButtonid);

    QString getTitle() const;
    void setTitle(const QString &newTitle);

    QString getSubtitle() const;
    void setSubtitle(const QString &newSubtitle);

    QString getContext() const;
    void setContext(const QString &newContext);

    QByteArray getImage() const;
    void setImage(const QByteArray &newImage);

    QDateTime getUpdatetime() const;
    void setUpdatetime(const QDateTime &newUpdatetime);

    int getShowmode() const;
    void setShowmode(int newShowmode);

private:
    int buttonid;
    QString title;
    QString subtitle;
    QString context;
    int showmode;
    QByteArray image;
    QDateTime updatetime;
};

#endif // RECORD_H

