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
	f0r_param_double delay_x, delay_y, wet, dry, mode, rotcol;
	unsigned int _width;
	unsigned int _height;
	
public:
	glitch1(unsigned int width, unsigned int height)
	{
		delay_x = .5;
		delay_y = .5;
		wet = .5;
		dry = .5;
		mode = 0;
		rotcol = 0;
		register_param(delay_x, "Delay X", "Delay X");
		register_param(delay_y, "Delay Y", "Delay Y");
		register_param(wet, "wet", "wet");
		register_param(dry, "dry", "dry");
		register_param(mode, "mode", "mode");
		register_param(rotcol, "rot.col.", "rot.col.");
		_width = width;
		_height = height;
	}
	~glitch1() {
	}

	virtual void update(double time,
	                    uint32_t* out,
                        const uint32_t* in) {
		unsigned char c;
		long delay;
		delay = 2 * ((delay_x - 0.5) + (delay_y - 0.5) * double(_height)) * double(_width);
		
		if (delay>0){
		  for (int i = 0; i < delay; i++) {
		    out[i]=in[i];
		  }
		} {
		  for (int i = size - 1; i > size - 1 + delay; i--) {
		    out[i]=in[i];
		  }
		}
		
		
		for (int i = abs(delay); i < size; i++) {
			px_t p_src, p_dest;
			if (delay > 0){
			  p_src.u = out[i-delay];
			  p_dest.u = in[i];
			}else{
			  p_src.u = out[size - 1 - i + delay];
			  p_dest.u = in[size - 1 - i];
			}
		
			// Color rotate
			switch(int(rotcol*6)){
			  case 0: // RGB
			    break;
			  case 1: // RBG
			    c=p_src.c[1];
			    p_src.c[1] = p_src.c[2];
			    p_src.c[2] = c;
			    break;
			  case 2: // GRB
			    c=p_src.c[1];
			    p_src.c[1] = p_src.c[0];
			    p_src.c[0] = c;
			    break;
			  case 3: // GBR
			    c=p_src.c[0];
			    p_src.c[0] = p_src.c[1];
			    p_src.c[1] = p_src.c[2];
			    p_src.c[2] = c;
			    break;
			  case 4: // BGR
			    c=p_src.c[2];
			    p_src.c[2] = p_src.c[0];
			    p_src.c[0] = c;
			    break;
			  default: // BRG
			    c=p_src.c[2];
			    p_src.c[2] = p_src.c[1];
			    p_src.c[1] = p_src.c[0];
			    p_src.c[0] = c;
			    break;
			}
			switch(int(mode*5)){
			  case 0:
			      // 4-channel = 1 channel per video component, bounded
			      p_src.c[0]=std::min(float(p_src.c[0])*wet + float(p_dest.c[0])*dry,255.0);
			      p_src.c[1]=std::min(float(p_src.c[1])*wet + float(p_dest.c[1])*dry,255.0);
			      p_src.c[2]=std::min(float(p_src.c[2])*wet + float(p_dest.c[2])*dry,255.0);
			      p_src.c[3]=std::min(float(p_src.c[3])*wet + float(p_dest.c[3])*dry,255.0);
			      break;
			  case 1:
			      // 4-channel = 1 channel per video component, unbounded
			      p_src.c[0]=float(p_src.c[0])*wet + float(p_dest.c[0])*dry;
			      p_src.c[1]=float(p_src.c[1])*wet + float(p_dest.c[1])*dry;
			      p_src.c[2]=float(p_src.c[2])*wet + float(p_dest.c[2])*dry;
			      p_src.c[3]=float(p_src.c[3])*wet + float(p_dest.c[3])*dry;
			      break;
			  case 2:
			      // XOR mode
			      p_src.c[0]=int(p_src.c[0] * wet) ^ int(p_dest.c[0] * dry);
			      p_src.c[1]=int(p_src.c[1] * wet) ^ int(p_dest.c[1] * dry);
			      p_src.c[2]=int(p_src.c[2] * wet) ^ int(p_dest.c[2] * dry);
			      p_src.c[3]=int(p_src.c[2] * wet) ^ int(p_dest.c[3] * dry);
			      break;
			  case 3:
			      // Handle as mono sound
			      p_src.u = p_src.u * wet + p_dest.u * dry;
			      break;
			  default:
			      // Handle as stereo sound
			      p_src.s[0]=p_src.s[0] * wet +p_dest.s[0] * dry;
			      p_src.s[1]=p_src.s[1] * wet +p_dest.s[1] * dry;
			      break;
			}
			if (delay > 0){
			  out[i] = p_src.u;
			}else{
			  out[size - 1 - i] = p_src.u;
			}
		  }
		  
	}
};


frei0r::construct<glitch1> plugin("glitch1",
									"Audio glitch delay",
									"D-j-a-y, vloop",
									0,2);

