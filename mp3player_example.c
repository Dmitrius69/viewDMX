int MP3Player::Init(const char *pFileName)

{

    mpg123_init();

    m_mpgHandle = mpg123_new(0, 0);
    if(mpg123_open(m_mpgHandle, pFileName) != MPG123_OK)
    {
        qFatal("Cannot open %s: %s", pFileName, mpg123_strerror(m_mpgHandle));
        return 0;
    }
}
 
int MP3Player::Play()

{

    unsigned char *audio;
    int mc;
    size_t bytes;
    qWarning("play_frame");


    static unsigned char* arr = 0;

    /* The first call will not decode anything but return MPG123_NEW_FORMAT! */

    mc = mpg123_decode_frame(m_mpgHandle, &m_framenum, &audio, &bytes);

    if(bytes)
    {

        /* Normal flushing of data, includes buffer decoding. */

        /*This function is my already implemented audio class which uses ALSA to output decoded audio to Sound Card*/
        if (m_audioPlayer.Play(arr,bytes) < (int)bytes) 
        {
            qFatal("Deep trouble! Cannot flush to my output anymore!");
        }

    }
    /* Special actions and errors. */
    if(mc != MPG123_OK)
    {
        if(mc == MPG123_ERR)
        {
            qFatal("...in decoding next frame: %s", mpg123_strerror(m_mpgHandle));
            return CSoundDecoder::EOFStream;

        }
        if(mc == MPG123_DONE)
        {
            return CSoundDecoder::EOFStream;
        }
        if(mc == MPG123_NO_SPACE)
        {
            qFatal("I have not enough output space? I didn't plan for this.");
            return CSoundDecoder::EOFStream;
        }
        if(mc == MPG123_NEW_FORMAT)
        {
            long iFrameRate;
            int encoding;
            mpg123_getformat(m_mpgHandle, &iFrameRate, &m_iChannels, &encoding);

            m_iBytesPerChannel = mpg123_encsize(encoding);

            if (m_iBytesPerChannel == 0)
                qFatal("bytes per channel is 0 !!");

            m_audioPlayer.Init(m_iChannels , iFrameRate , m_iBytesPerChannel);

        }
    }
