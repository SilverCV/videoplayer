#include "widget.h"
#include "ui_widget.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QDebug>
Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
    connect(ui->Play,SIGNAL(clicked()),this,SLOT(Play()));
    connect(ui->pushButton,SIGNAL(clicked()),this,SLOT(open()));
}

Widget::~Widget()
{
    delete ui;
}
void Widget::open()
{
    QString filter = tr("mp4 file(*.mp4 *.avi *.rmvb)");
    filepath_ = QFileDialog::getOpenFileName(this,tr("选择文件"),QString(),filter);
}
void Widget::Play()
{
    if (filepath_.isEmpty())
    {
        QMessageBox::warning(nullptr,tr("warning"),tr("没有选择文件"),QMessageBox::Cancel);
        return;
    }
    char input_file[1024];
    strcpy(input_file,filepath_.toStdString().data());
    av_register_all();
    AVFormatContext *pFormatCtx = avformat_alloc_context();
    if (avformat_open_input(&pFormatCtx,input_file,nullptr,nullptr) != 0)
    {
        printf("Could not find the input stream\n");
        return;
    }
    if (avformat_find_stream_info(pFormatCtx,nullptr) < 0)
    {
        printf("Could not find the stream info\n");
        return;
    }
    av_dump_format(pFormatCtx,0,input_file,0);

    int video_stream_index = -1;
    for (int var = 0; var < pFormatCtx->nb_streams; ++var) {
        if (pFormatCtx->streams[var]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            video_stream_index = var;
            break;
        }
    }
    if (video_stream_index==-1)
    {
        printf("Could not find the video stream");
    }

    AVCodecContext *pCodecCtx = pFormatCtx->streams[video_stream_index]->codec;
    AVCodec *pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
    if (pCodec == NULL)
    {
        printf("Could not find the decodec\n");
        return;
    }
    if (avcodec_open2(pCodecCtx,pCodec,nullptr) < 0)
    {
        printf("Could not open the decoder\n");
        return;
    }
    AVFrame *pFrame = av_frame_alloc();
    AVFrame *pFrameRGB = av_frame_alloc();
    unsigned char *buffer = (unsigned char*)av_malloc(av_image_get_buffer_size(AV_PIX_FMT_RGB32,pCodecCtx->width,pCodecCtx->height,1));
    av_image_fill_arrays(pFrameRGB->data,pFrameRGB->linesize,buffer,AV_PIX_FMT_RGB32,pCodecCtx->width,pCodecCtx->height,1);
    AVPacket *pkt = av_packet_alloc();
    av_init_packet(pkt);
    struct SwsContext *pSwsCtx = sws_alloc_context();


    pSwsCtx = sws_getContext(pCodecCtx->width,pCodecCtx->height,pCodecCtx->pix_fmt,
                             pCodecCtx->width,pCodecCtx->height,AV_PIX_FMT_RGB32,SWS_BICUBIC,
                             nullptr,nullptr,nullptr);

    while (av_read_frame(pFormatCtx,pkt) >= 0) {
        if (pkt->stream_index == video_stream_index)
        {
            int result = avcodec_send_packet(pCodecCtx,pkt);
            if (result != 0)
            {
                printf("could not send the packet to codec context\n");
                return;
            }
            while(avcodec_receive_frame(pCodecCtx,pFrame) == 0)
            {
                sws_scale(pSwsCtx,(const uchar* const *)pFrame->data,pFrame->linesize,0,pFrame->height,
                          pFrameRGB->data,pFrameRGB->linesize);
                QImage image = QImage((uchar*)pFrameRGB->data[0],pCodecCtx->width,pCodecCtx->height,
                        QImage::Format_RGB32);
                ui->Canvas->setPixmap(QPixmap::fromImage(image));
                Delay(1000);

            }
        }
        av_free_packet(pkt);

    }

    sws_freeContext(pSwsCtx);
    av_packet_free(&pkt);
    av_free(buffer);
    avformat_close_input(&pFormatCtx);
}
void Widget::Delay(int delay)
{
    QTime time = QTime::currentTime().addMSecs(delay);
    if (QTime::currentTime() < time)
        QCoreApplication::processEvents(QEventLoop::AllEvents,100);
}
