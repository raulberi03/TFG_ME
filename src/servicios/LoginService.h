#ifndef LOGIN_SERVICE_H
#define LOGIN_SERVICE_H

class LoginService {
public:
    // Simula la autenticación contra un servidor (por ahora siempre devuelve true)
    bool login(const char* username, const char* password) {
        // Aquí iría la lógica de conexión al servidor
        return true;
    }
};

#endif // LOGIN_SERVICE_H
