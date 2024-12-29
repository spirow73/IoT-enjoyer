-- init.sql
-- Crear la base de datos
CREATE DATABASE gym_db;

-- Conectar a la base de datos `gym_db`
\c gym_db

-- Crear tabla `users` (Usuarios del gimnasio)
CREATE TABLE IF NOT EXISTS users (
    id SERIAL PRIMARY KEY,               -- Identificador único del usuario
    first_name VARCHAR(100) NOT NULL,    -- Nombre del usuario
    last_name VARCHAR(100) NOT NULL,     -- Apellido del usuario
    username VARCHAR(100) UNIQUE NOT NULL, -- Nombre de usuario único
    rfid_tag VARCHAR(100) UNIQUE,        -- Identificador de la tarjeta RFID (opcional)
    created_at TIMESTAMP DEFAULT NOW(),  -- Fecha de creación del usuario
    updated_at TIMESTAMP DEFAULT NOW()   -- Fecha de última actualización
);

-- Crear tabla `machines` (Máquinas del gimnasio)
CREATE TABLE IF NOT EXISTS machines (
    id SERIAL PRIMARY KEY,               -- Identificador único de la máquina
    name VARCHAR(100) NOT NULL,          -- Nombre de la máquina (ej.: "Cinta de correr")
    description TEXT,                    -- Descripción opcional de la máquina
    is_in_use BOOLEAN DEFAULT FALSE,     -- Indicador de si la máquina está en uso
    is_active BOOLEAN DEFAULT TRUE,      -- Indicador de si la máquina está activa (no en mantenimiento)
    created_at TIMESTAMP DEFAULT NOW(),  -- Fecha de creación de la máquina
    updated_at TIMESTAMP DEFAULT NOW()   -- Fecha de última actualización
);

-- Crear tabla `user_machine_sessions` (Registro de uso de máquinas por usuarios)
CREATE TABLE IF NOT EXISTS user_machine_sessions (
    id SERIAL PRIMARY KEY,               -- Identificador único de la sesión
    user_id INT NOT NULL REFERENCES users(id) ON DELETE CASCADE, -- Relación con la tabla users
    machine_id INT NOT NULL REFERENCES machines(id) ON DELETE CASCADE, -- Relación con la tabla machines
    start_time TIMESTAMP DEFAULT NOW(),  -- Inicio de la sesión
    end_time TIMESTAMP,                  -- Fin de la sesión (opcional mientras esté en uso)
    is_active BOOLEAN DEFAULT TRUE,      -- Indicador de si la sesión está activa
    created_at TIMESTAMP DEFAULT NOW()   -- Fecha de creación del registro
);

-- Trigger para evitar que una máquina en mantenimiento se marque como en uso
CREATE OR REPLACE FUNCTION prevent_use_while_inactive()
RETURNS TRIGGER AS $$
BEGIN
    IF NEW.is_in_use = TRUE AND NEW.is_active = FALSE THEN
        RAISE EXCEPTION 'Cannot use a machine that is under maintenance';
    END IF;
    RETURN NEW;
END;
$$ LANGUAGE plpgsql;

-- Crear el trigger para validar el estado de las máquinas antes de una actualización
CREATE TRIGGER check_machine_status
BEFORE UPDATE ON machines
FOR EACH ROW
EXECUTE FUNCTION prevent_use_while_inactive();

-- Crear tabla `user_checkins` (Opcional, para gestionar entradas y salidas al gimnasio)
CREATE TABLE IF NOT EXISTS user_checkins (
    id SERIAL PRIMARY KEY,               -- Identificador único del registro
    user_id INT NOT NULL REFERENCES users(id) ON DELETE CASCADE, -- Relación con usuarios
    checkin_time TIMESTAMP DEFAULT NOW(),-- Hora de entrada al gimnasio
    checkout_time TIMESTAMP              -- Hora de salida del gimnasio
);

-- Insertar datos iniciales en las tablas
-- Ejemplo de usuarios
INSERT INTO users (first_name, last_name, username, rfid_tag)
VALUES 
('Juan', 'Perez', 'jhonny', '845ADC79'),
('Alvaro', 'Martin', 'alvmart', '4A27FBA9');


-- Ejemplo de máquinas
INSERT INTO machines (name, description)
VALUES 
('Treadmill', 'Running machine'),
('Bench Press', 'Strength training bench'),
('Rowing Machine', 'Full-body cardio machine');

