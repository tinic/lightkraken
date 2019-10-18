#ifndef SPI_H
#define SPI_H

namespace lightguy {

class SPI_0 {
public:
	static SPI_0 &instance();

	void transfer(const uint8_t *buf, size_t len);
    void update();

private:
	bool initialized = false;
	void init();

    void dma_setup(const uint8_t *buf, size_t len);
	const uint8_t *cbuf = 0;
	size_t clen = 0;
    bool active = false;
    bool scheduled = false;
};

class SPI_2 {
public:
	static SPI_2 &instance();

	void transfer(const uint8_t *buf, size_t len);
    void update();

private:
    bool initialized = false;
	void init();

    void dma_setup(const uint8_t *buf, size_t len);
	const uint8_t *cbuf = 0;
	size_t clen = 0;
    bool active = false;
    bool scheduled = false;
};

}

#endif  // #ifndef SPI_H
