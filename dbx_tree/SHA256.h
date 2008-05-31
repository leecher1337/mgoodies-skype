#include "stdint.h"


class SHA256
{
public:
	typedef uint32_t THash[8];

	SHA256();
	~SHA256();

	void SHAInit();
	void SHAUpdate(void * Data, uint32_t Length);
	void SHAFinal(THash & Hash);
private:
	THash m_Hash;
	uint64_t m_Length; /// Datalength in byte

	uint8_t m_Block[64];

	void SHABlock();
	void Swap64(void * Addr);
};
