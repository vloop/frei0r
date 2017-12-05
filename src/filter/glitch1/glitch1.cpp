/* 
 * glitch1
 * 2017 D-j-a-y, vloop
 *
 * This source code  is free software; you can  redistribute it and/or
 * modify it under the terms of the GNU Public License as published by
 * the Free Software  Foundation; either version 3 of  the License, or
 * (at your option) any later version.
 *
 * This source code is distributed in the hope that it will be useful,
 * but  WITHOUT ANY  WARRANTY; without  even the  implied  warranty of
 * MERCHANTABILITY or FITNESS FOR  A PARTICULAR PURPOSE.  Please refer
 * to the GNU Public License for more details.
 *
 * You should  have received  a copy of  the GNU Public  License along
 * with this source code; if  not, write to: Free Software Foundation,
 * Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */
#include "frei0r.hpp"

#include <algorithm>
#include <vector>
#include <utility>
#include <cassert>

#include <iostream>

union px_t {
	uint32_t u;
	int16_t s[2]; // Fake "stereo"
	unsigned char c[4]; // 0=B, 1=G,2=R,3=A ? i think :P
};

class glitch1 : public frei0r::filter {
private:
	f0r_param_double delay_x, delay_y, wet, dry;
	unsigned int _width;
	unsigned int _height;
	
public:
	glitch1(unsigned int width, unsigned int height)
	{
		delay_x = 1;
		delay_y = 1;
		wet = .5;
		dry = .5;
		register_param(delay_x, "Delay X", "Delay X");
		register_param(delay_y, "Delay Y", "Delay Y");
		register_param(wet, "wet", "wet");
		register_param(dry, "dry", "dry");
		_width = width;
		_height = height;
	}
	~glitch1() {
	}

	virtual void update(double time,
	                    uint32_t* out,
                        const uint32_t* in) {
		unsigned char c;
		unsigned long delay;
		// delay = (unsigned long)((f0r_param_double)size * factor);
		delay = long(delay_x * double(_width))+long(delay_y * double(_height)) * _width;
		for (int i = 0; i < delay; i++) {
		  out[i]=in[i];
		}
		for (int i = delay; i < size; i++) {
			px_t pi, pi2;
			pi.u = out[i-delay];
			pi2.u = in[i];
			/* Color rotate
			c=pi.c[0];
			pi.c[0] = pi.c[1];
			pi.c[1] = pi.c[2];
			pi.c[2] = c;
			*/
			/* XOR mode
			pi.c[0]=pi.c[0] ^ pi2.c[0];
			pi.c[1]=pi.c[1] ^ pi2.c[1];
			pi.c[2]=pi.c[2] ^ pi2.c[2];
			*/
			/* 4-channel = 1 channel per video component 
			pi.c[0]=std::min(float(pi.c[0])*wet + float(pi2.c[0])*dry,255.0);
			pi.c[1]=std::min(float(pi.c[1])*wet + float(pi2.c[1])*dry,255.0);
			pi.c[2]=std::min(float(pi.c[2])*wet + float(pi2.c[2])*dry,255.0);
			pi.c[3]=std::min(float(pi.c[3])*wet + float(pi2.c[3])*dry,255.0);
                        */
			// Handle as mono sound
			pi.u = pi.u * wet + pi2.u * dry;
			/* Handle as stereo sound
			pi.s[0]=pi.s[0]+pi2.s[0];
			pi.s[1]=pi.s[1]+pi2.s[1];
			*/
			out[i] = pi.u;
		}
	}
};


frei0r::construct<glitch1> plugin("glitch1",
									"Audio glitch delay",
									"D-j-a-y, vloop",
									0,2);

