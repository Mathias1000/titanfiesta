class CCrc32Dynamic
{
public:
	CCrc32Dynamic()  : m_pdwCrc32Table(NULL) {};
	~CCrc32Dynamic() { Free(); };

	void Init(void)
	{
		dword dwPolynomial = 0xEDB88320;
		dword i, j;

		Free();
		m_pdwCrc32Table = new dword[256];

		dword dwCrc;
		for(i = 0; i < 256; i++)
		{
			dwCrc = i;
			for(j = 8; j > 0; j--)
			{
				if(dwCrc & 1)
					dwCrc = (dwCrc >> 1) ^ dwPolynomial;
				else
					dwCrc >>= 1;
			}
			m_pdwCrc32Table[i] = dwCrc;
		}
	}

	void Free(void)
	{
		delete m_pdwCrc32Table;
		m_pdwCrc32Table = NULL;
	}


	dword DoCRC32( byte* dwMemStart, dword dwMemSize ) const
	{
		dword dwErrorCode = 0;
		dword dwCrc32 = 0xFFFFFFFF;
		try{
			if(m_pdwCrc32Table == NULL)
				throw 0;

			for(dword i = 0; i < dwMemSize; i++){
				CalcCrc32(dwMemStart[i], dwCrc32);
			}

			dwCrc32 = ~dwCrc32;
		}catch(...){
			dwCrc32 = 0;
		}

		return dwCrc32;
	}

protected:
	dword *m_pdwCrc32Table;
	inline void CalcCrc32(const byte curByte, dword &dwCrc32) const
	{
		dwCrc32 = ((dwCrc32) >> 8) ^ m_pdwCrc32Table[(curByte) ^ ((dwCrc32) & 0x000000FF)];
	}
};