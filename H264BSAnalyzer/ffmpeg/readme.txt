ffmpeg�汾��2.6.3
��̬��

mingw�������
./configure --prefix=../ffmpeg-2.6.3-bin --enable-shared --disable-static --enable-w32threads --disable-debug \
--enable-decoder=h264,hevc,mpeg4,mjpeg  --enable-demuxer=h264,hevc,avi --enable-muxer=h264,hevc,avi,mp4,mjpeg \
--enable-parser=h264,hevc,mjpeg --enable-protocol=file \
--disable-hwaccels  \
--extra-libs=-lmsvcrt



��̬����룺
./configure --prefix=../ffmpeg-2.6.3-bin \
--disable-shared --enable-static \
--disable-debug  --disable-everything \
--enable-memalign-hack --enable-gpl --disable-network \
--enable-encoder=bmp,mjpeg,mpeg4 \
--disable-encoder=h263 \
--enable-decoder=h264,hevc,mpeg4,mjpeg,bmp  \
--disable-decoder=h263 \
--enable-demuxer=h264,hevc,avi \
--enable-muxer=h264,hevc,avi,mp4,mjpeg \
--enable-parser=h264,hevc,mjpeg \
--disable-parser=h263 \
--enable-protocol=file \
--enable-filter=scale \
--disable-indevs \
--disable-outdevs \
--disable-hwaccels \
--disable-bsfs \
--disable-doc \
--enable-w32threads  --extra-libs=-lmsvcrt

(ע�⣺���ϱ���Ŀ���Ա����̽�����С����ı���)

