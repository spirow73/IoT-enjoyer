import express from "express";
import machineRoutes from "./routes/machines.js";
import userRoutes from "./routes/users.js";
import sessionRoutes from "./routes/sessions.js";

import pool from "./db/dbConnection.js"; // Importar conexión para inicializarla

const app = express();
const PORT = 3000;

// Middleware para parsear JSON
app.use(express.json());

// Registrar las rutas
app.use("/machines", machineRoutes);
app.use("/users", userRoutes);
app.use("/sessions", sessionRoutes);

// Endpoint raíz para verificar el servidor
app.get("/", (req, res) => {
  res.send("API está funcionando");
});

// Probar conexión antes de levantar el servidor
pool.query("SELECT 1")
  .then(() => {
    console.log("Base de datos conectada. Iniciando servidor...");
    app.listen(PORT, () => {
      console.log(`Servidor corriendo en http://localhost:${PORT}`);
    });
  })
  .catch(err => {
    console.error("No se pudo conectar a la base de datos:", err.message);
    process.exit(1);
  });
