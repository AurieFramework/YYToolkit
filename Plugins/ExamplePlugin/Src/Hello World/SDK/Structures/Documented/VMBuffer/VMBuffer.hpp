#pragma once
struct VMBuffer
{
	void** vTable;
	int m_size;
	int m_numLocalVarsUsed;
	int m_numArguments;
	char* m_pBuffer;
	void** m_pConvertedBuffer;
	char* m_pJumpBuffer;
};