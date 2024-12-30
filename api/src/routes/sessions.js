import express from "express";
import pool from "../db/dbConnection.js";

const router = express.Router();

// Registrar el acceso de un usuario a una máquina
router.post("/", async (req, res) => {
  const { rfid_tag, machine_id, session_status } = req.body;

  if (session_status === "start") {
    // Lógica para iniciar sesión
    try {
      const query = `
        INSERT INTO user_machine_sessions (rfid_tag, machine_id, start_time, is_active)
        VALUES ($1, $2, NOW(), TRUE)
        RETURNING *;
      `;
      const values = [rfid_tag, machine_id];
      const result = await pool.query(query, values);

      return res.status(201).json({
        status: "success",
        message: "Sesión iniciada exitosamente",
        data: result.rows[0]
      });
    } catch (error) {
      console.error("Error al iniciar sesión:", error.message);
      return res.status(500).json({ error: "Error al iniciar sesión" });
    }
  } else if (session_status === "end") {
    // Lógica para cerrar sesión
    try {
      const query = `
        UPDATE user_machine_sessions
        SET end_time = NOW(), is_active = FALSE
        WHERE rfid_tag = $1 AND is_active = TRUE
        RETURNING *;
      `;
      const values = [rfid_tag];
      const result = await pool.query(query, values);

      if (result.rows.length === 0) {
        return res.status(404).json({ error: "Sesión activa no encontrada" });
      }

      return res.status(200).json({
        status: "success",
        message: "Sesión finalizada exitosamente",
        data: result.rows[0]
      });
    } catch (error) {
      console.error("Error al cerrar sesión:", error.message);
      return res.status(500).json({ error: "Error al cerrar sesión" });
    }
  } else {
    return res.status(400).json({ error: "Estado de sesión no válido" });
  }
});

// Obtener todas las sesiones de un usuario
router.get("/user/:rfid_tag", async (req, res) => {
  const { rfid_tag } = req.params;

  try {
    const query = `
      SELECT u.first_name, u.last_name, m.name AS machine_name,
             s.start_time, s.end_time, s.is_active
      FROM user_machine_sessions s
      JOIN users u ON s.rfid_tag = u.rfid_tag
      JOIN machines m ON s.machine_id = m.id
      WHERE s.rfid_tag = $1;
    `;
    const values = [rfid_tag];
    const result = await pool.query(query, values);

    res.status(200).json({
      status: "success",
      data: result.rows,
    });
  } catch (error) {
    console.error("Error al obtener sesiones del usuario:", error.message);
    res.status(500).json({ error: "Error al obtener las sesiones del usuario" });
  }
});

// Obtener todas las sesiones activas
router.get("/active", async (req, res) => {
  try {
    const query = `
      SELECT u.first_name, u.last_name, m.name AS machine_name,
             s.start_time
      FROM user_machine_sessions s
      JOIN users u ON s.rfid_tag = u.rfid_tag
      JOIN machines m ON s.machine_id = m.id
      WHERE s.is_active = TRUE;
    `;
    const result = await pool.query(query);

    res.status(200).json({
      status: "success",
      data: result.rows,
    });
  } catch (error) {
    console.error("Error al obtener sesiones activas:", error.message);
    res.status(500).json({ error: "Error al obtener las sesiones activas" });
  }
});

// Obtener el tiempo promedio de uso de una máquina por un usuario
router.get("/average", async (req, res) => {
  const { rfid_tag, machine_id } = req.query; // Ahora usamos rfid_tag en lugar de user_id

  if (!rfid_tag || !machine_id) {
      return res.status(400).json({ error: "rfid_tag y machine_id son requeridos" });
  }

  try {
      const query = `
          SELECT AVG(EXTRACT(EPOCH FROM (end_time - start_time))) AS average_time_seconds
          FROM user_machine_sessions
          WHERE rfid_tag = $1 AND machine_id = $2 AND end_time IS NOT NULL;
      `;
      const values = [rfid_tag, machine_id];
      const result = await pool.query(query, values);

      if (result.rows[0].average_time_seconds === null) {
          return res.status(404).json({ error: "No hay datos suficientes para calcular la media" });
      }

      res.status(200).json({
          status: "success",
          rfid_tag,
          machine_id,
          average_time_seconds: result.rows[0].average_time_seconds,
      });
  } catch (error) {
      console.error("Error al calcular tiempo promedio:", error.message);
      res.status(500).json({ error: "Error al calcular tiempo promedio" });
  }
});

export default router;
