import express from "express";
import pool from "../db/dbConnection.js";

const router = express.Router();

// Obtener el tiempo promedio de uso de una máquina por un usuario
router.get("/user-machine-average", async (req, res) => {
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