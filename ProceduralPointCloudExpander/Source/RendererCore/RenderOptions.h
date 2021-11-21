//
// Created by Niskp on 08/10/2021.
//

#ifndef PAG_RENDEROPTIONS_H
#define PAG_RENDEROPTIONS_H

namespace PPCX {
	enum paramShader {
		posicion,
		color
	};

	static unsigned numParamShader() {
		return color + 1;
	};

	static unsigned anchoVentanaPorDefecto = 1280;
	static unsigned altoVentanaPorDefecto = 720;

};

#endif //PAG_RENDEROPTIONS_H
