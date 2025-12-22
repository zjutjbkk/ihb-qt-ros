
#ifndef IHB_H
#define IHB_H

#include <QMainWindow>


QT_BEGIN_NAMESPACE
namespace Ui { class IHB; }
QT_END_NAMESPACE



class IHB : public QMainWindow

{
    Q_OBJECT

public:
    IHB(QWidget *parent = nullptr);
    ~IHB();


private:
    Ui::IHB *ui;

    //按键监听
    void keyPressEvent(QKeyEvent *event);
};

#endif // IHB_H
