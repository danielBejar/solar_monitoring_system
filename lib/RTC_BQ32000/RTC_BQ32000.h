/************************************************************************/
/*  Autor: Francisco Timez
	Año: 2015
	Esta Libreria reune algunas funciones para la implementacion del 
	RTC de Texas Instrument BQ32000.
	Ademas de la calse Fecha_Hora el cual permite hacer una lectura
	rapida de todos los datos proporcionados por el RTC.
/************************************************************************/
//Direccion del RTC en I2C = 0B11010000 ultimo bit de 1=lectura o 0=escritura

//Algunas Clases//////////////////////////////////////////////////////////////////////////
//
#include <inttypes.h>
#include <Arduino.h>

static const String Meses[] = {"Ene","Feb","Mar","Abr","May","Jun","Jul","Ago","Sep","Oct","Nov","Dic"};
static const uint8_t daysInMonth [] = {31,28,31,30,31,30,31,31,30,31,30,31};

static uint8_t char2int(const char* p) {
	uint8_t v = 0;
	if ('0' <= *p && *p <= '9')
	v = *p - '0';
	return 10 * v + *++p - '0';
}
// Simple general-purpose date/time class (no TZ / DST / leap second handling!)
class Fecha_Hora {
	public:
	Fecha_Hora (long t =0);
	Fecha_Hora (const char* date, const char* time);
	Fecha_Hora (uint16_t anho, uint8_t mes, uint8_t dia, 
							uint8_t hora =0, uint8_t min =0, uint8_t seg =0);
	Fecha_Hora (uint16_t con_anho, uint8_t con_mes, uint8_t con_dia,
						uint8_t con_dow, uint8_t con_hora, uint8_t con_min, uint8_t con_sec);	

	uint16_t year() const       { return 2000 + a_add; }
	uint8_t month() const       { return m; }
	uint8_t day() const         { return d; }
	uint8_t hour() const        { return hh; }
	uint8_t minute() const      { return mm; }
	uint8_t second() const      { return ss; }
	uint8_t DofW() const		{ return dow;}		
	String StrTime() const		{	String time;
									if (hh<10)
										time = "0";
									time = String(time + hh + ":");
									if (mm<10)
										time = String(time + "0");
									time = String(time + mm + ":");
									if (ss<10)
									time = String(time + "0");
									time = String(time + ss);
									return time;}
	String StrDate() const		{	String date;
									if (d<10)
										date = "0";
									date = String(date + d + " ");
									date = String(date + Meses[m-1] + " ");
									date = String(date + (2000 + a_add));
									return date;}
	void Validar(void);
	
	//protected:
	int8_t a_add, m, d, dow, hh, mm, ss;
	
	private:
	uint8_t DiaDeSemana() const;
};

Fecha_Hora::Fecha_Hora (long t) {
	ss = t % 60;
	t /= 60;
	mm = t % 60;
	t /= 60;
	hh = t % 24;
	uint16_t days = t / 24;
	uint8_t leap;
	for (a_add = 0; ; ++a_add) {
		leap = a_add % 4 == 0;
		if (days < 365 + leap)
		break;
		days -= 365 + leap;
	}
	for (m = 1; ; ++m) {
		uint8_t daysPerMonth = daysInMonth[m-1];
		if (leap && m == 2)
		++daysPerMonth;
		if (days < daysPerMonth)
		break;
		days -= daysPerMonth;
	}
	d = days + 1;
}

// A convenient constructor for using "the compiler's time":
//   Fecha_Hora now (__DATE__, __TIME__);
// NOTE: using PSTR would further reduce the RAM footprint
Fecha_Hora::Fecha_Hora (const char* date, const char* time) {
	// sample input: date = "Dec 26 2009", time = "12:34:56"
	a_add = char2int(date + 9);
	// Jan Feb Mar Apr May Jun Jul Aug Sep Oct Nov Dec
	switch (date[0]) {
		case 'J': m = date[1] == 'a' ? 1 : m = date[2] == 'n' ? 6 : 7; break;
		case 'F': m = 2; break;
		case 'A': m = date[2] == 'r' ? 4 : 8; break;
		case 'M': m = date[2] == 'r' ? 3 : 5; break;
		case 'S': m = 9; break;
		case 'O': m = 10; break;
		case 'N': m = 11; break;
		case 'D': m = 12; break;
	}
	d = char2int(date + 4);
	hh = char2int(time);
	mm = char2int(time + 3);
	ss = char2int(time + 6);
}

Fecha_Hora::Fecha_Hora (uint16_t con_anho, uint8_t con_mes, uint8_t con_dia,
						uint8_t con_dow, uint8_t con_hora, uint8_t con_min, uint8_t con_sec)
{
	if (con_anho >= 2000)
	con_anho -= 2000;
	a_add = con_anho;
	m = con_mes;
	d = con_dia;
	hh = con_hora;
	mm = con_min;
	ss = con_sec;
	dow = con_dow;
	return;
}

Fecha_Hora::Fecha_Hora (uint16_t con_anho, uint8_t con_mes, uint8_t con_dia,
							uint8_t con_hora, uint8_t con_min, uint8_t con_sec) {
	if (con_anho >= 2000)
	con_anho -= 2000;
	a_add = con_anho;
	m = con_mes;
	d = con_dia;
	hh = con_hora;
	mm = con_min;
	ss = con_sec;
	dow = DiaDeSemana();
	return;
}

uint8_t Fecha_Hora::DiaDeSemana() const
{
	uint8_t y = a_add;
	if (y >= 2000)
	y -= 2000;
	uint16_t days = d;
	for (uint8_t i = 1; i < m; ++i)
	days += daysInMonth[i-1];
	if (m > 2 && y % 4 == 0)
	++days;
	days = days + 365 * y + (y + 3) / 4 - 1;
	uint16_t day = (((days * 24L + hh) * 60 + m) * 60 + ss) / 86400L;
	return (day + 6) % 7; // Jan 1, 2000 is a Saturday, i.e. returns 6
}

void Fecha_Hora::Validar(void)
{
	int diasdelmes [] = {31,28,31,30,31,30,31,31,30,31,30,31};
	if (ss > 59) ss -= 60;
	if (ss < 0) ss += 60;
	if (mm > 59) mm -= 60;
	if (mm < 0) mm += 60;
	if (hh > 23) hh -= 24;
	if (hh < 0) hh += 24;
	if (d > diasdelmes[m-1]) d -= diasdelmes[m-1];
	if (d < 1 ) d += diasdelmes[m-1];
	if (m > 12) m -= 12;
	if (m < 1) m +=12;
	if (a_add > 99) a_add -= 100;
	if (a_add < 0) a_add += 100;
	

		
	
}

//////////////////////////////////////////////////////////////////////////
#define BQ32000_ADDRESS		0B1101000
#define BQ32000_CAL_CFG1	0x07
#define BQ32000__CAL_S		0x05

String Dia_text[] {"Domingo","Lunes","Martes","Miercoles",
	"Jueves","Viernes","Sabado"};
String Mes_text[] {"Enero","Febrero","Marzo","Abril","Mayo","Junio",
	"Julio","Agosto","Septiembre","Octubre","Noviembre","Diciembre"};

class BQ32000
{
	public:
		BQ32000(void) {};
		BQ32000(const Fecha_Hora& FyH);
		static void Guardar(uint16_t con_anho, uint8_t con_mes, uint8_t con_dia,
		uint8_t con_dow, uint8_t con_hora, uint8_t con_min, uint8_t con_sec);
		static Fecha_Hora Leer();
		static void Calibracion(int8_t valor);
		static uint8_t readRegister(uint8_t address);
		static uint8_t writeRegister(uint8_t address, uint8_t value);
		static uint8_t bcd2bin (uint8_t val) { return val - 6 * (val >> 4); }
		static uint8_t bin2bcd (uint8_t val) { return val + 6 * (val / 10); }
		
};
BQ32000::BQ32000(const Fecha_Hora& FyH)
{
	Guardar(FyH.year(),FyH.month(),FyH.day(),FyH.DofW(),FyH.hour(),FyH.minute(),FyH.second());
	return;
}
void BQ32000::Guardar(uint16_t con_anho, uint8_t con_mes, uint8_t con_dia,
								uint8_t con_dow, uint8_t con_hora, uint8_t con_min, uint8_t con_sec)
{
	
	Wire.beginTransmission(BQ32000_ADDRESS);
	Wire.write((byte) 0);
	Wire.write(bin2bcd(con_sec));
	Wire.write(bin2bcd(con_min));
	Wire.write(bin2bcd(con_hora));
	Wire.write(bin2bcd(con_dow));
	Wire.write(bin2bcd(con_dia));
	Wire.write(bin2bcd(con_mes));
	Wire.write(bin2bcd(con_anho - 2000));
	Wire.endTransmission();
}
Fecha_Hora BQ32000::Leer()
{
	Wire.beginTransmission(BQ32000_ADDRESS);
	Wire.write((byte) 0);
	Wire.endTransmission();

	Wire.requestFrom(BQ32000_ADDRESS, 7);
	uint8_t ss = bcd2bin(Wire.read() & 0B01111111);
	uint8_t mm = bcd2bin(Wire.read() & 0B01111111);
	uint8_t hh = bcd2bin(Wire.read() & 0B00111111);
	uint8_t dow = bcd2bin(Wire.read() & 0B00000111);
	uint8_t d = bcd2bin(Wire.read() & 0B00111111);
	uint8_t m = bcd2bin(Wire.read() & 0B00011111);
	uint16_t y = bcd2bin(Wire.read())+2000;
	dow |= 0x07;

	return Fecha_Hora (y, m, d, dow, hh, mm, ss);
}
void BQ32000::Calibracion(int8_t valor)
{
	/* Sets the calibration value to given value in the range -31 - 31, which
     * corresponds to -126ppm - +63ppm; see table 13 in th BQ32000 datasheet.
     */
    uint8_t val;
    if (valor > 31) valor = 31;
    if (valor < -31) valor = -31;
    val = (uint8_t) (valor < 0) ? -valor | (1<<BQ32000__CAL_S) : valor;
    val |= readRegister(BQ32000_CAL_CFG1) & ~0x3f;
    writeRegister(BQ32000_CAL_CFG1, val);
}
uint8_t BQ32000::readRegister(uint8_t address)
{
	/* Read and return the value in the register at the given address.*/
    Wire.beginTransmission(BQ32000_ADDRESS);
    Wire.write((byte) address);
    Wire.endTransmission();
    Wire.requestFrom(BQ32000_ADDRESS, 1);
    return Wire.read();
}
uint8_t BQ32000::writeRegister(uint8_t address, uint8_t value)
{
	/* Write the given value to the register at the given address.*/
    Wire.beginTransmission(BQ32000_ADDRESS);
    Wire.write(address);
    Wire.write(value);
    Wire.endTransmission();
}