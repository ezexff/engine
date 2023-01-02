struct win32_state
{
    uint64 TotalSize;
    void *GameMemoryBlock;
    /*
    win32_replay_buffer ReplayBuffers[4];
    
    HANDLE RecordingHandle; // файл для записи цикла
    int InputRecordingIndex; // 0 - цикл не записан, 1 - цикл записан

    HANDLE PlaybackHandle; // файл для проигрывания цикла
    int InputPlayingIndex; // 0 - не проигрывать цикл, 1 - проигрывать цикл
    
    char EXEFileName[WIN32_STATE_FILE_NAME_COUNT];
    char *OnePastLastEXEFileNameSlash;
    */
};