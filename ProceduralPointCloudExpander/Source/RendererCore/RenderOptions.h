//
// Created by Niskp on 08/10/2021.
//

#ifndef PAG_RENDEROPTIONS_H
#define PAG_RENDEROPTIONS_H

namespace PAG {
	enum modoDibujado {
		nubePuntos,
		wireframe,
		mallaTriangulos
	};

	static unsigned numModosDibujo() {
		return modoDibujado::mallaTriangulos + 1;
	}

	enum paramShader {
		posicion,
		color
	};

	static unsigned numParamShader() {
		return color + 1;
	};

	static unsigned anchoVentanaPorDefecto = 1024;
	static unsigned altoVentanaPorDefecto = 576;

};

#endif //PAG_RENDEROPTIONS_H
