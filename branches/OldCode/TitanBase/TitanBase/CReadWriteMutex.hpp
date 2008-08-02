class ReadWriteMutex : boost::noncopyable
{
public:
    ReadWriteMutex() :
        m_readers(0),
        m_pendingWriters(0),
        m_currentWriter(false)
    {}

    class ScopedReadLock : boost::noncopyable
    {
    public:
        ScopedReadLock(ReadWriteMutex& rwLock) :
            m_rwLock(rwLock)
        {
            m_rwLock.acquireReadLock();
        }

        ~ScopedReadLock()
        {
            m_rwLock.releaseReadLock();
        }

    private:
        ReadWriteMutex& m_rwLock;
    };

    class ScopedWriteLock : boost::noncopyable
    {
    public:
        ScopedWriteLock(ReadWriteMutex& rwLock) :
            m_rwLock(rwLock)
        {
            m_rwLock.acquireWriteLock();
        }

        ~ScopedWriteLock()
        {
            m_rwLock.releaseWriteLock();
        }

    private:
        ReadWriteMutex& m_rwLock;
    };

    void acquireReadLock()
    {
        boost::mutex::scoped_lock lock(m_mutex);

        while(m_pendingWriters != 0 || m_currentWriter)
        {
            m_writerFinished.wait(lock);
        }
        ++m_readers;
    }

    void releaseReadLock()
    {
        boost::mutex::scoped_lock lock(m_mutex);
        --m_readers;

        if(m_readers == 0)
        {
            m_noReaders.notify_all();
        }
    }

    void acquireWriteLock()
    {
        boost::mutex::scoped_lock lock(m_mutex);

        ++m_pendingWriters;
        
        while(m_readers > 0)
        {
            m_noReaders.wait(lock);
        }

        while(m_currentWriter)
        {
            m_writerFinished.wait(lock);
        }
        --m_pendingWriters;
        m_currentWriter = true;
    }

    void releaseWriteLock()
    {        
        boost::mutex::scoped_lock lock(m_mutex);
        m_currentWriter = false;
        m_writerFinished.notify_all();
    }

private:
    boost::mutex m_mutex;

    dword m_readers;
    boost::condition m_noReaders;

    dword m_pendingWriters;
    bool m_currentWriter;
    boost::condition m_writerFinished;
};
