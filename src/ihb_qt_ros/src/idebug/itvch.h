#ifndef ITVCH_H
#define ITVCH_H

#include <QWidget>

namespace Ui {
class ITVCH;
}

class ITVCH : public QWidget
{
    Q_OBJECT

public:
    explicit ITVCH(QWidget *parent = nullptr, int Num = 0, int Value = 0);
    ~ITVCH();
    void update(int Value);
    void setCh(int ch);
    int chNum;
    int chValue;

private slots:
    void on_chNum_textChanged(const QString &arg1);

    void on_chNum_editingFinished();

    void on_chValue_editingFinished();

    void on_horizontalSliderValue_valueChanged(int value);

private:
    Ui::ITVCH *ui;

    QString* chData;
    void update(int Num, int Value);
    void sendData();
};

#endif // ITVCH_H
