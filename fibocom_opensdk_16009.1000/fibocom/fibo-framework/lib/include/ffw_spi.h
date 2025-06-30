#ifndef LIB_INCLUDE_SPI
#define LIB_INCLUDE_SPI

typedef int INT32;


typedef enum
{
    SPI_WORK_STANDARD,
    SPI_WORK_SYNC,
    SPI_WORK_S8,
    SPI_WORK_3_WIRE,
    SPI_WORK_SLAVE,
} drvSpiWorkMode_t;

typedef enum
{
    CLK_SOURCE_RTC32,
    CLK_SOURCE_XTAL26,
    CLK_SOURCE_RC26_26,
    CLK_SOURCE_GNSSPLL_133m,
    CLK_SOURCE_APLL_167,
} drvClkSource_t;

typedef enum
{
    SPI_CS_ACTIVE_HIGH,
    SPI_CS_ACTIVE_LOW,
} drvSpiCsPol;

typedef enum
{
    SPI_CPHA_1Edge,
    SPI_CPHA_2Edge,
} drvSpiCphaPol;

typedef enum
{
    SPI_DI_0 = 0,
    SPI_DI_1,
    SPI_DI_2,
} drvSpiInputSel;

typedef enum
{
    SPI_DIRECT_POLLING = 0,
    SPI_DIRECT_IRQ,
    SPI_DMA_POLLING,
    SPI_DMA_IRQ,
} drvSpiTransferMode;

typedef enum
{
    SPI_CPOL_LOW = 0,
    SPI_CPOL_HIGH,
} drvSpiCpolPol;

typedef enum
{
    SPI_I2C_CS0 = 0,
    SPI_I2C_CS1,
    SPI_I2C_CS2,
    SPI_I2C_CS3,
#ifdef CONFIG_FIBOCOM_BASE
    SPI_GPIO_CS,
#endif
} drvSpiCsSel;

typedef struct
{
    bool inputEn;
#ifdef CONFIG_SOC_8850
    bool lsb;
#endif
    uint8_t oe_delay;
    uint32_t name;
    uint32_t framesize;
    uint32_t baud;
#ifdef CONFIG_SOC_8850
    drvSpiWorkMode_t work_mode;
    drvClkSource_t clk_source;
#endif
    drvSpiCsPol cs_polarity0;
    drvSpiCsPol cs_polarity1;
    drvSpiCpolPol cpol;
    drvSpiCphaPol cpha;
    drvSpiInputSel input_sel;
    drvSpiTransferMode transmode;
#ifdef CONFIG_SOC_8850
    uint8_t rxf_full_thrhld;
    uint8_t rxf_empty_thrhld;
    uint8_t txf_full_thrhld;
    uint8_t txf_empty_thrhld;
    uint32_t spi_tx_data_len;
    uint32_t spi_rx_data_len;
#endif
} drvSpiConfig_t;

typedef struct
{
    uint32_t bits_per_word;
    uint32_t speed_hz;
    drvSpiCsSel cs;
    drvSpiCsPol cs_polarity;
    bool lsb;
}SPI_IOC;

#define SPIHANDLE uint32_t


INT32 ffw_spi_open(drvSpiConfig_t *cfg, SPIHANDLE *spiFd);

INT32 ffw_spi_close(SPIHANDLE *spiFd);

INT32 ffw_spi_send(SPIHANDLE spiFd, SPI_IOC spiIoc, void *sendaddr, uint32_t size);

INT32 ffw_spi_recv(SPIHANDLE spiFd, SPI_IOC spiIoc, void *readaddr, uint32_t size);

INT32 ffw_spi_send_rcv(SPIHANDLE spiFd, SPI_IOC spiIoc, void *sendaddr, void *readaddr, uint32_t size);

extern INT32 fibo_hal_spi_open(drvSpiConfig_t *cfg, SPIHANDLE *spiFd);
extern INT32 fibo_hal_spi_close(SPIHANDLE *spiFd);
extern INT32 fibo_hal_spi_send(SPIHANDLE spiFd, SPI_IOC spiIoc, void *sendaddr, uint32_t size);
extern INT32 fibo_hal_spi_recv(SPIHANDLE spiFd, SPI_IOC spiIoc, void *readaddr, uint32_t size);
extern INT32 fibo_hal_spi_send_rcv(SPIHANDLE spiFd, SPI_IOC spiIoc, void *sendaddr, void *readaddr, uint32_t size);

#endif
