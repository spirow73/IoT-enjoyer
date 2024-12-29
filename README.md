# 💪 **Sistema IoT para Gimnasios Inteligentes**

¡Bienvenido al futuro de los gimnasios! Nuestro proyecto combina la tecnología IoT con una experiencia única para los usuarios y administradores de gimnasios, optimizando el monitoreo, gestión y control del espacio.

## 🚀 **Descripción del Proyecto**
Este sistema IoT transforma los gimnasios tradicionales en espacios inteligentes y eficientes mediante la integración de monitoreo en tiempo real, automatización y análisis de datos. Con una arquitectura basada en nodos IoT, este proyecto permite:
- **Monitoreo de Máquinas de Ejercicio**: Detectar el uso de las máquinas en tiempo real, identificar usuarios y registrar datos como tiempo de uso y peso levantado.
- **Gestión de Entrada/Salida**: Control de acceso de los usuarios mediante tarjetas RFID.
- **Control de Iluminación Ambiental**: Ajustar automáticamente la intensidad y el color de la luz según las condiciones lumínicas externas para una mejor experiencia.

## 🛠️ **Características Principales**
1. **Automatización Completa**:
   - Sensores PIR para monitorear el estado de las máquinas.
   - Lectores RFID para identificar usuarios en las máquinas y al entrar/salir del gimnasio.
2. **Visualización y Control**:
   - Dashboard interactivo en Node-RED para monitoreo en tiempo real.
   - Visualización de datos históricos en ThingSpeak.
3. **Eficiencia Energética**:
   - Iluminación ajustable basada en un sensor LDR y controlada por un LED RGB.

## 🧩 **Arquitectura del Proyecto**
El sistema está compuesto por tres nodos IoT:
1. **Nodo 1: Monitoreo de Máquinas**:
   - Detecta si las máquinas están en uso y quién las utiliza.
2. **Nodo 2: Gestión de Entrada/Salida**:
   - Registra accesos y asocia rutinas a los usuarios.
3. **Nodo 3: Control de Iluminación**:
   - Optimiza la iluminación del gimnasio automáticamente.

Los nodos se comunican utilizando **MQTT**, mientras que los datos son visualizados y gestionados mediante **Node-RED** y **ThingSpeak**.

## 👥 **Equipo de Desarrollo**
Este proyecto ha sido desarrollado por un equipo apasionado y comprometido:
- **Juan Antonio Moya** - Especialista en hardware y configuración de nodos.
- **Juan Pérez** - Líder de desarrollo y gestión de la comunicación MQTT.
- **Jorge Jimenez** - Responsable de integración con Node-RED y análisis en ThingSpeak.

## 🛒 **¿Por qué Elegir Nuestro Sistema?**
1. **Eficiencia Operativa**:
   - Los administradores de gimnasios pueden monitorear el uso de las máquinas y la asistencia en tiempo real.
2. **Mejora de la Experiencia del Usuario**:
   - Iluminación personalizada y una experiencia fluida gracias a la tecnología RFID.
3. **Innovación y Futuro**:
   - Aprovecha las últimas tecnologías IoT para crear un gimnasio del futuro, hoy.

## 🏗️ **Requerimientos Técnicos**
- **Hardware**:
  - 3x NodeMCU ESP8266
  - 3x Lectores RFID (MFRC522)
  - 2x Sensores PIR
  - 1x Sensor de luz (LDR)
  - 1x LED RGB
- **Software**:
  - Mosquitto MQTT Broker
  - Node-RED para flujos de datos
  - ThingSpeak para análisis y reportes
  - Arduino IDE o PlatformIO para programación

## 📬 **Contacto**
¿Interesado en implementar esta solución en tu gimnasio? ¡Contáctanos!  
Correo: equipoiotgimnasios@ejemplo.com

---

¡Gracias por tu interés en nuestro proyecto!
