class CPacket
{
public:
	CPacket():size(0),command(0){buffer = NULL;}
	CPacket(char* nBuf):size(0),command(0){buffer = nBuf;}
	~CPacket(){}

	void CreatePacket(dword command){
		size = 1;
		pos = 1;
		this->command = command;
	}

	void DecryptPacket(){
		size = *reinterpret_cast<byte*>(buffer);
		//command = *reinterpret_cast<dword*>(buffer + 2);
		pos = 1;
	}

	void EncryptPacket(){
		//*reinterpret_cast<byte*>(buffer) = size - 1;
		//*reinterpret_cast<word*>(buffer + 2) = command;
	}

	template <class T> T Read(){
		pos += sizeof(T);
		return *reinterpret_cast<T*>(buffer + pos - sizeof(T));
	}

	template <class T> void Add(T val){
		*reinterpret_cast<T*>(buffer + pos) = val;
		pos += sizeof(T);
		size += sizeof(T);
	}

	void AddString(char* str){
		for(dword i = 0; i < strlen(str); i++)
			Add<char>(str[i]);
		//Add<char>(0);
	}

	word size;
	word command;
	char* buffer;

protected:
	dword pos;
};