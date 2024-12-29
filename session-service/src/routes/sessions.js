import express from "express";
import pool from "../db/dbConnection.js";

const router = express.Router();

// Obtener la media de tiempo de uso de una máquina por un usuario
router.get("/average-time/:userId/:machineId", async (req, res) => {
  const { userId, machineId } = req.params;

  try {
    const query = `
      SELECT AVG(EXTRACT(EPOCH FROM (end_time - start_time))) AS average_time_seconds
      FROM user_machine_sessions
      WHERE user_id = $1 AND machine_id = $2 AND end_time IS NOT NULL;
    `;
    const values = [userId, machineId];

    const result = await pool.query(query, values);

    if (result.rows[0].average_time_seconds === null) {
      return res.status(404).json({
        error: "No hay sesiones registradas con tiempos de finalización para este usuario y máquina.",
      });
    }

    res.status(200).json({
      user_id: userId,
      machine_id: machineId,
      average_time_seconds: result.rows[0].average_time_seconds,
      average_time_minutes: result.rows[0].average_time_seconds / 60,
    });
  } catch (error) {
    console.error("Error al calcular la media de tiempo:", error.message);
    res.status(500).json({ error: "Error al calcular la media de tiempo" });
  }
});

export default router;
