//
// Created by Niskp on 31/10/2021.
//

#include <stdafx.h>
#include "MaterialManager.h"

PPCX::MaterialManager *PPCX::MaterialManager::instancia = nullptr;

/**
 * Consulta del objeto único de la clase
 * @return Puntero al MaterialManager
 */
PPCX::MaterialManager *PPCX::MaterialManager::getInstancia() {
	if (!instancia) {
		instancia = new MaterialManager;
	}
	return instancia;
}

PPCX::MaterialManager::MaterialManager() {

}

/**
 * Destructor. Se encarga de destruir todos los materiales que se encuentren dentro del mapa "materiales"
 */
PPCX::MaterialManager::~MaterialManager() {
	for (auto &material: materiales) {
		delete material.second;
	}
}

/**
 * Añade un nuevo material al mapa
 * @param nombreMaterial nombre con el que se guardará
 * @param material a guardar
 */
void PPCX::MaterialManager::nuevoMaterial(const std::string &nombreMaterial, PPCX::Material *material) {
	materiales.insert(std::make_pair(nombreMaterial, material));
}


PPCX::Material *PPCX::MaterialManager::getMaterial(const std::string &nombreMaterial) {
	auto mat = materiales.find(nombreMaterial);
	if (mat != materiales.end()) {
		return mat->second;
	} else {
		throw std::runtime_error(
				"[MaterialManager]: No se ha encontrado ningun material con el nombre " + nombreMaterial);
	}
}


