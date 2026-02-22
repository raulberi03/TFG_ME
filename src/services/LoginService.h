#ifndef LOGIN_SERVICE_H
#define LOGIN_SERVICE_H

class LoginService {
public:
    // Simula la autenticacion contra un servidor (por ahora siempre devuelve true).
    bool login(const char* username, const char* password) {
        (void)username;
        (void)password;
        return true;
    }
};

#endif // LOGIN_SERVICE_H
