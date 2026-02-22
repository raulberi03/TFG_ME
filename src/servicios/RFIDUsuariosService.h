#pragma once
#include <Arduino.h>

class RFIDUsuariosService {
public:
    static constexpr size_t MAX_USUARIOS = 20;

    bool agregarSiNoTiene(const String& userId, const String& uid) {
        int idx = buscarUsuario(userId);
        if (idx >= 0) {
            return false;
        }
        int libre = buscarLibre();
        if (libre < 0) {
            return false;
        }
        entries[libre].inUse = true;
        entries[libre].userId = userId;
        entries[libre].uid = uid;
        return true;
    }

    bool sobrescribir(const String& userId, const String& uid) {
        int idx = buscarUsuario(userId);
        if (idx < 0) {
            return false;
        }
        entries[idx].uid = uid;
        return true;
    }

    bool desvincular(const String& userId) {
        int idx = buscarUsuario(userId);
        if (idx < 0) {
            return false;
        }
        entries[idx].inUse = false;
        entries[idx].userId = "";
        entries[idx].uid = "";
        return true;
    }

    bool tieneUsuario(const String& userId) const {
        return buscarUsuarioConst(userId) >= 0;
    }

    String uidDeUsuario(const String& userId) const {
        int idx = buscarUsuarioConst(userId);
        if (idx < 0) {
            return "";
        }
        return entries[idx].uid;
    }

private:
    struct Entry {
        String userId;
        String uid;
        bool inUse = false;
    };

    Entry entries[MAX_USUARIOS];

    int buscarUsuario(const String& userId) {
        for (size_t i = 0; i < MAX_USUARIOS; ++i) {
            if (entries[i].inUse && entries[i].userId == userId) {
                return (int)i;
            }
        }
        return -1;
    }

    int buscarUsuarioConst(const String& userId) const {
        for (size_t i = 0; i < MAX_USUARIOS; ++i) {
            if (entries[i].inUse && entries[i].userId == userId) {
                return (int)i;
            }
        }
        return -1;
    }

    int buscarLibre() {
        for (size_t i = 0; i < MAX_USUARIOS; ++i) {
            if (!entries[i].inUse) {
                return (int)i;
            }
        }
        return -1;
    }
};
