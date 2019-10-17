#ifndef SPI_H
#define SPI_H

namespace lightguy {

class SPI_0 {
public:
	static SPI_0 &instance();

	void dma_transfer(const uint8_t *buf, size_t len);

private:
	bool initialized = false;
	void init();

	const uint8_t *cbuf = 0;
	size_t clen = 0;
};

class SPI_2 {
public:
	static SPI_2 &instance();

	void dma_transfer(const uint8_t *buf, size_t len);

private:
	bool initialized = false;
	void init();

	const uint8_t *cbuf = 0;
	size_t clen = 0;
};

}

#endif  // #ifndef SPI_H
