#ifndef ICINPUTMETHODKEYBOARD_H
#define ICINPUTMETHODKEYBOARD_H

#include <QDialog>
#include <QPlainTextEdit>
#include <QSignalMapper>

namespace Ui {
class ICInputMethodKeyboard;
}

class ICInputMethodKeyboard : public QDialog
{
    Q_OBJECT

public:
    explicit ICInputMethodKeyboard(QWidget *parent = 0);
    ~ICInputMethodKeyboard();

    void SetTextEditor(QPlainTextEdit* editor) { editor_ = editor;}

protected:
    void changeEvent(QEvent *e);
    void keyPressEvent(QKeyEvent* e);

private slots:
    void on_btn_ent_clicked();
    void OnInputButtonClicked(const QString& text);
    void ShowMaching_(const QStringList& texts);


    void on_btn_sw_clicked();

    void on_btn_bs_clicked();

    void on_btn_space_clicked();

    void on_nextGroup_clicked();

    void on_upGroup_clicked();

    void OnCnButtonClicked();

private:
    bool IsChEn_() const;
    Ui::ICInputMethodKeyboard *ui;
    QPlainTextEdit* editor_;
    QSignalMapper signalMapper_;
    QList<QPushButton*> cnButtons;

};

#endif // ICINPUTMETHODKEYBOARD_H
