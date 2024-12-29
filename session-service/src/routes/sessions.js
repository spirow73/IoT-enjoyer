import express from "express";
import pool from "../db/dbConnection.js";

const router = express.Router();

// Registrar el acceso de un usuario a una máquina
router.post("/", async (req, res) => {
  const { rfid_tag, machine_id } = req.body;

  try {
    const query = `
      INSERT INTO user_machine_sessions (rfid_tag, machine_id, start_time, is_active)
      VALUES ($1, $2, NOW(), TRUE)
      RETURNING *;
    `;
    const values = [rfid_tag, machine_id];
    const result = await pool.query(query, values);

    res.status(201).json({
      status: "success",
      message: "Sesión iniciada exitosamente",
      data: result.rows[0],
    });
  } catch (error) {
    console.error("Error al iniciar sesión en máquina:", error.message);
    if (error.code === "23505") {
      return res
        .status(400)
        .json({ error: "Ya existe una sesión activa para este usuario en otra máquina" });
    }
    res.status(500).json({ error: "Error al iniciar sesión en máquina" });
  }
});

// Registrar el fin de una sesión activa
router.patch("/:session_id", async (req, res) => {
  const { session_id } = req.params;

  try {
    const query = `
      UPDATE user_machine_sessions
      SET end_time = NOW(), is_active = FALSE
      WHERE id = $1 AND is_active = TRUE
      RETURNING *;
    `;
    const values = [session_id];
    const result = await pool.query(query, values);

    if (result.rows.length === 0) {
      return res.status(404).json({ error: "Sesión activa no encontrada o ya finalizada" });
    }

    res.status(200).json({
      status: "success",
      message: "Sesión finalizada exitosamente",
      data: result.rows[0],
    });
  } catch (error) {
    console.error("Error al finalizar sesión en máquina:", error.message);
    res.status(500).json({ error: "Error al finalizar la sesión" });
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

export default router;
