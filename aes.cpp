#include "aes.h"
//----------------------------------------------------------------------------------------------------------------------------------------
AES::AES(size_t key_size, uint8_t* key)
{
	R = new uint8_t[4];
	R[0] = 0x02;
	R[1] = 0x00;
	R[2] = 0x00;
	R[3] = 0x00;
		
	w = aes_init(key_size);
	aes_key_expansion(key, w);
}
//----------------------------------------------------------------------------------------------------------------------------------------
AES::~AES()
{
	delete[]w;
	delete[]R;
}
//----------------------------------------------------------------------------------------------------------------------------------------
int AES::Encrypt(size_t size_in, uint8_t* in, uint8_t* out)
{
	if (size_in % 16 != 0) return 1;

	int i = 0;
	while (i < size_in)
	{
		aes_cipher(in+i, out+i);
		i+= 16;
	}
	return 0;
}
//----------------------------------------------------------------------------------------------------------------------------------------
int AES::Decrypt(size_t size_in, size_t size_out, uint8_t* in, uint8_t* out)
{
	if (size_in % 16 != 0) return 1;
	uint8_t temp[16];

	if (size_out == 0)size_out = size_in;

	int i = 0;
	while (i < size_out)
	{
		aes_inv_cipher(in + i, temp);
		int j = 0;
		while ((j < 16) && (i + j < size_out))
		{
			out[i + j] = temp[j];
			j++;
		}
		i += 16;
	}
	return 0;
}
//----------------------------------------------------------------------------------------------------------------------------------------
int AES::DecryptFileOnDisk(const char* szFileNameIn, const char* szFileNameOut,size_t size_out)
{
	std::fstream f;
	f.open(szFileNameIn, std::ios::in | std::ios::binary);
	std::vector<unsigned char> buffer(std::istreambuf_iterator<char>(f), {});
	f.close();
	

	size_t N = buffer.size();
	if (N % 16 != 0)return 1;
	if (size_out == 0) size_out = N;

	char* szBuferEncrypted = new char[N];
	char* szBuferDecrypted = new char[size_out];
	for (size_t i = 0; i < buffer.size(); i++)szBuferEncrypted[i] = buffer[i];
	
	int res = Decrypt(N, size_out,(uint8_t*)szBuferEncrypted, (uint8_t*)szBuferDecrypted);
	
	f.open(szFileNameOut, (std::ios::out | std::ios::binary));
	for (int i = 0; i<size_out; i++)f << szBuferDecrypted[i];
	f.close();


	delete[]szBuferDecrypted;
	delete[]szBuferEncrypted;
	return 0;
}
//----------------------------------------------------------------------------------------------------------------------------------------
size_t AES::EncryptFileOnDisk(const char* szFileNameIn, const char* szFileNameOut)
{
	std::fstream f;
	f.open(szFileNameIn, std::ios::in | std::ios::binary);
	std::vector<unsigned char> buffer(std::istreambuf_iterator<char>(f), {});
	f.close();

	size_t N = buffer.size();if (N % 16 != 0) N += 16 - (N % 16);

	char* szBuferEncrypted = new char[N];
	char* szBuferDecrypted = new char[N];
	for (size_t i = 0; i < buffer.size(); i++)szBuferDecrypted[i] = buffer[i];

	int res = Encrypt(N, (uint8_t*)szBuferDecrypted, (uint8_t*)szBuferEncrypted);

	f.open(szFileNameOut, std::ios::out | std::ios::binary);
	for (int i = 0; i<N; i++)f << szBuferEncrypted[i];
	f.close();

	delete[]szBuferDecrypted;
	delete[]szBuferEncrypted;

	return buffer.size();
}
//----------------------------------------------------------------------------------------------------------------------------------------
uint8_t AES::gadd(uint8_t a, uint8_t b) {return a^b;}
uint8_t AES::gsub(uint8_t a, uint8_t b) {return a^b;}
uint8_t AES::gmult(uint8_t a, uint8_t b)
{
	uint8_t p = 0, i = 0, hbs = 0;
	for (i = 0; i < 8; i++) 
	{
		if (b & 1) p ^= a;
		hbs = a & 0x80;
		a <<= 1;
		if (hbs) a ^= 0x1b; // 0000 0001 0001 1011	
		b >>= 1;
	}
	return (uint8_t)p;
}
//----------------------------------------------------------------------------------------------------------------------------------------
void AES::coef_add(uint8_t a[], uint8_t b[], uint8_t d[])
{

	d[0] = a[0]^b[0];
	d[1] = a[1]^b[1];
	d[2] = a[2]^b[2];
	d[3] = a[3]^b[3];
}
//----------------------------------------------------------------------------------------------------------------------------------------
void AES::coef_mult(uint8_t *a, uint8_t *b, uint8_t *d) {

	d[0] = gmult(a[0],b[0])^gmult(a[3],b[1])^gmult(a[2],b[2])^gmult(a[1],b[3]);
	d[1] = gmult(a[1],b[0])^gmult(a[0],b[1])^gmult(a[3],b[2])^gmult(a[2],b[3]);
	d[2] = gmult(a[2],b[0])^gmult(a[1],b[1])^gmult(a[0],b[2])^gmult(a[3],b[3]);
	d[3] = gmult(a[3],b[0])^gmult(a[2],b[1])^gmult(a[1],b[2])^gmult(a[0],b[3]);
}
//----------------------------------------------------------------------------------------------------------------------------------------
uint8_t* AES::Rcon(uint8_t i)
{
	
	if (i == 1) {
		R[0] = 0x01; // x^(1-1) = x^0 = 1
	} else if (i > 1) {
		R[0] = 0x02;
		i--;
		while (i-1 > 0) {
			R[0] = gmult(R[0], 0x02);
			i--;
		}
	}
	
	return R;
}
//----------------------------------------------------------------------------------------------------------------------------------------
void AES::add_round_key(uint8_t *state, uint8_t *w, uint8_t r) {
	
	uint8_t c;
	
	for (c = 0; c < Nb; c++) {
		state[Nb*0+c] = state[Nb*0+c]^w[4*Nb*r+4*c+0];   //debug, so it works for Nb !=4 
		state[Nb*1+c] = state[Nb*1+c]^w[4*Nb*r+4*c+1];
		state[Nb*2+c] = state[Nb*2+c]^w[4*Nb*r+4*c+2];
		state[Nb*3+c] = state[Nb*3+c]^w[4*Nb*r+4*c+3];	
	}
}
//----------------------------------------------------------------------------------------------------------------------------------------
void AES::mix_columns(uint8_t *state) {

	uint8_t a[] = {0x02, 0x01, 0x01, 0x03}; // a(x) = {02} + {01}x + {01}x2 + {03}x3
	uint8_t i, j, col[4], res[4];

	for (j = 0; j < Nb; j++) {
		for (i = 0; i < 4; i++) {
			col[i] = state[Nb*i+j];
		}

		coef_mult(a, col, res);

		for (i = 0; i < 4; i++) {
			state[Nb*i+j] = res[i];
		}
	}
}
//----------------------------------------------------------------------------------------------------------------------------------------
void AES::inv_mix_columns(uint8_t *state) {

	uint8_t a[] = {0x0e, 0x09, 0x0d, 0x0b}; // a(x) = {0e} + {09}x + {0d}x2 + {0b}x3
	uint8_t i, j, col[4], res[4];

	for (j = 0; j < Nb; j++) {
		for (i = 0; i < 4; i++) {
			col[i] = state[Nb*i+j];
		}

		coef_mult(a, col, res);

		for (i = 0; i < 4; i++) {
			state[Nb*i+j] = res[i];
		}
	}
}
//----------------------------------------------------------------------------------------------------------------------------------------
void AES::shift_rows(uint8_t *state) {

	uint8_t i, k, s, tmp;

	for (i = 1; i < 4; i++) {
		// shift(1,4)=1; shift(2,4)=2; shift(3,4)=3
		// shift(r, 4) = r;
		s = 0;
		while (s < i) {
			tmp = state[Nb*i+0];
			
			for (k = 1; k < Nb; k++) {
				state[Nb*i+k-1] = state[Nb*i+k];
			}

			state[Nb*i+Nb-1] = tmp;
			s++;
		}
	}
}
//----------------------------------------------------------------------------------------------------------------------------------------
void AES::inv_shift_rows(uint8_t *state) {

	uint8_t i, k, s, tmp;

	for (i = 1; i < 4; i++) {
		s = 0;
		while (s < i) {
			tmp = state[Nb*i+Nb-1];
			
			for (k = Nb-1; k > 0; k--) {
				state[Nb*i+k] = state[Nb*i+k-1];
			}

			state[Nb*i+0] = tmp;
			s++;
		}
	}
}
//----------------------------------------------------------------------------------------------------------------------------------------
void AES::sub_bytes(uint8_t *state) {

	uint8_t i, j;
	uint8_t row, col;

	for (i = 0; i < 4; i++) {
		for (j = 0; j < Nb; j++) {
			row = (state[Nb*i+j] & 0xf0) >> 4;
			col = state[Nb*i+j] & 0x0f;
			state[Nb*i+j] = s_box[16*row+col];
		}
	}
}
//----------------------------------------------------------------------------------------------------------------------------------------
void AES::inv_sub_bytes(uint8_t *state) {

	uint8_t i, j;
	uint8_t row, col;

	for (i = 0; i < 4; i++) {
		for (j = 0; j < Nb; j++) {
			row = (state[Nb*i+j] & 0xf0) >> 4;
			col = state[Nb*i+j] & 0x0f;
			state[Nb*i+j] = inv_s_box[16*row+col];
		}
	}
}
//----------------------------------------------------------------------------------------------------------------------------------------
void AES::sub_word(uint8_t *w) {

	uint8_t i;

	for (i = 0; i < 4; i++) {
		w[i] = s_box[16*((w[i] & 0xf0) >> 4) + (w[i] & 0x0f)];
	}
}
//----------------------------------------------------------------------------------------------------------------------------------------
void AES::rot_word(uint8_t *w) {

	uint8_t tmp;
	uint8_t i;

	tmp = w[0];

	for (i = 0; i < 3; i++) {
		w[i] = w[i+1];
	}

	w[3] = tmp;
}
//----------------------------------------------------------------------------------------------------------------------------------------
void AES::aes_key_expansion(uint8_t *key, uint8_t *w) {

	uint8_t tmp[4];
	uint8_t i, j;
	uint8_t len = Nb*(Nr+1);

	for (i = 0; i < Nk; i++) {
		w[4*i+0] = key[4*i+0];
		w[4*i+1] = key[4*i+1];
		w[4*i+2] = key[4*i+2];
		w[4*i+3] = key[4*i+3];
	}

	for (i = Nk; i < len; i++) {
		tmp[0] = w[4*(i-1)+0];
		tmp[1] = w[4*(i-1)+1];
		tmp[2] = w[4*(i-1)+2];
		tmp[3] = w[4*(i-1)+3];

		if (i%Nk == 0) {

			rot_word(tmp);
			sub_word(tmp);
			coef_add(tmp, Rcon(i/Nk), tmp);

		} else if (Nk > 6 && i%Nk == 4) {

			sub_word(tmp);

		}

		w[4*i+0] = w[4*(i-Nk)+0]^tmp[0];
		w[4*i+1] = w[4*(i-Nk)+1]^tmp[1];
		w[4*i+2] = w[4*(i-Nk)+2]^tmp[2];
		w[4*i+3] = w[4*(i-Nk)+3]^tmp[3];
	}
}
//----------------------------------------------------------------------------------------------------------------------------------------
uint8_t* AES::aes_init(size_t key_size)
{

        switch (key_size) {
		default:
		case 16: Nk = 4; Nr = 10; break;
		case 24: Nk = 6; Nr = 12; break;
		case 32: Nk = 8; Nr = 14; break;
	}

	return new uint8_t[(Nb*(Nr+1)*4)];
}
//----------------------------------------------------------------------------------------------------------------------------------------
void AES::aes_cipher(uint8_t *in, uint8_t *out)
{

	uint8_t state[4*Nb];
	uint8_t r, i, j;

	for (i = 0; i < 4; i++) {
		for (j = 0; j < Nb; j++) {
			state[Nb*i+j] = in[i+4*j];
		}
	}

	add_round_key(state, w, 0);

	for (r = 1; r < Nr; r++) {
		sub_bytes(state);
		shift_rows(state);
		mix_columns(state);
		add_round_key(state, w, r);
	}

	sub_bytes(state);
	shift_rows(state);
	add_round_key(state, w, Nr);

	for (i = 0; i < 4; i++) {
		for (j = 0; j < Nb; j++) {
			out[i+4*j] = state[Nb*i+j];
		}
	}
}
//----------------------------------------------------------------------------------------------------------------------------------------
void AES::aes_inv_cipher(uint8_t *in, uint8_t *out)
{

	uint8_t state[4*Nb];
	uint8_t r, i, j;

	for (i = 0; i < 4; i++) {
		for (j = 0; j < Nb; j++) {
			state[Nb*i+j] = in[i+4*j];
		}
	}

	add_round_key(state, w, Nr);

	for (r = Nr-1; r >= 1; r--) {
		inv_shift_rows(state);
		inv_sub_bytes(state);
		add_round_key(state, w, r);
		inv_mix_columns(state);
	}

	inv_shift_rows(state);
	inv_sub_bytes(state);
	add_round_key(state, w, 0);

	for (i = 0; i < 4; i++) {
		for (j = 0; j < Nb; j++) {
			out[i+4*j] = state[Nb*i+j];
		}
	}
}
//----------------------------------------------------------------------------------------------------------------------------------------
