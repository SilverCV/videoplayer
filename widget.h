#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QTime>
#ifdef __cplusplus
extern "C" {
#endif
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
#ifdef __cplusplus
}
#endif
QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

    void Delay(int delay); //延迟播放
private slots:
    void Play();
    void open();

private:
    Ui::Widget *ui;
    QString filepath_;
};
#endif // WIDGET_H
