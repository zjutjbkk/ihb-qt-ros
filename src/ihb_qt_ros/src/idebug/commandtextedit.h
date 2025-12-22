#ifndef COMMANDTEXTEDIT_H
#define COMMANDTEXTEDIT_H

#include <QPlainTextEdit>
#include <QWidget>
#include <QObject>
class CommandTextEdit : public QPlainTextEdit
// 父对象(QWidget *parent = nullptr);
{
    Q_OBJECT
public:
    CommandTextEdit(QWidget *parent = nullptr);
    void start(bool circle,int time);
    void stop();

private slots:
    void runItem();
private:
    bool __circle = false;
    bool __stop = false;
    bool __wait = false;
    int __time = 0;
    int currentActionRow = 0;
    void sendData(QString indata);
signals:
    void finishRunSignal();
};

#endif // COMMANDTEXTEDIT_H
