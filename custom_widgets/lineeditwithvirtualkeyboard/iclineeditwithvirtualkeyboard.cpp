#include "iclineeditwithvirtualkeyboard.h"

#include "virtualkeyboarddialog.h"
#include <QMouseEvent>

ICLineEditWithVirtualKeyboard::ICLineEditWithVirtualKeyboard(QWidget * parent)
    : QLineEdit(parent)
{
    this->setReadOnly(true);
    virtualKeyboardDialog_ = VirtualKeyboardDialog::Instance();

    //virtualKeyboardDialog->ResetDisplay(this->text());
}

ICLineEditWithVirtualKeyboard::~ICLineEditWithVirtualKeyboard()
{
}

void ICLineEditWithVirtualKeyboard::mouseReleaseEvent(QMouseEvent *e)
{
//    virtualKeyboardDialog_->disconnect();
//    connect(virtualKeyboardDialog_,
//            SIGNAL(EnterComplete(QString)),
//            this,
//            SLOT(SetCurrentText(QString)));

    this->setStyleSheet("background:lightgreen;");
    virtualKeyboardDialog_->ResetDisplay();

    if(virtualKeyboardDialog_->exec() == QDialog::Accepted)
    {
//    disconnect(virtualKeyboardDialog_,
//               SIGNAL(EnterComplete(QString)),
//               this,
//               SLOT(SetCurrentText(QString)));
        SetCurrentText(virtualKeyboardDialog_->GetCurrentText());
    }
//    QLineEdit::mouseReleaseEvent(e);
    this->setStyleSheet("");
    e->accept();
}

void ICLineEditWithVirtualKeyboard::SetCurrentText(const QString &currentText)
{
    this->setText(currentText);
}
