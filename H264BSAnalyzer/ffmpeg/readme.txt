ffmpeg�汾��2.6.3
��̬��

mingw�������
./configure --prefix=../ffmpeg-2.6.3-bin --enable-shared --disable-static --enable-w32threads --disable-debug \
--enable-decoder=h264,hevc,mpeg4,mjpeg  --enable-demuxer=h264,hevc,avi --enable-muxer=h264,hevc,avi,mp4,mjpeg \
--enable-parser=h264,hevc,mjpeg --enable-protocol=file \
--disable-vaapi --disable-vdpau  --disable-dxva2 \
--extra-libs=-lmsvcrt



��̬����룺
./configure --prefix=../ffmpeg-2.6.3-bin \
--disable-shared --enable-static \
--enable-w32threads --disable-debug --disable-everything \
--enable-memalign-hack --enable-gpl --disable-network \
--enable-encoder=bmp,mjpeg,jpeg2000,mpeg4 \
--enable-decoder=h264,hevc,mpeg4,mjpeg,bmp  \
--enable-demuxer=h264,hevc,avi,matroska,image2,image_bmp_pipe \
--enable-muxer=h264,hevc,avi,matroska,mp4,mjpeg,image2 \
--enable-parser=h264,hevc,mjpeg,bmp \
--enable-protocol=file \
--enable-filter=scale \
--disable-indevs \
--disable-vaapi --disable-vdpau  --disable-dxva2 \
--extra-libs=-lmsvcrt

(ע�⣺���ϱ���Ŀ���Ա����̽�����С����ı���)

