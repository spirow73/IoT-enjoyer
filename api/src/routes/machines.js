import express from "express";
import pool from "../db/dbConnection.js";

const router = express.Router();

// Obtener todas las máquinas
router.get("/", async (req, res) => {
  try {
    const query = "SELECT * FROM machines;";
    const result = await pool.query(query);

    res.status(200).json(result.rows);
  } catch (error) {
    console.error("Error al obtener máquinas:", error.message);
    res.status(500).json({ error: "Error al obtener las máquinas" });
  }
});

// Obtener una máquina por ID
router.get("/:id", async (req, res) => {
  const { id } = req.params;

  try {
    const query = "SELECT * FROM machines WHERE id = $1;";
    const values = [id];
    const result = await pool.query(query, values);

    if (result.rows.length === 0) {
      return res.status(404).json({ error: "Máquina no encontrada" });
    }

    res.status(200).json(result.rows[0]);
  } catch (error) {
    console.error("Error al obtener máquina:", error.message);
    res.status(500).json({ error: "Error al obtener la máquina" });
  }
});

// Crear una nueva máquina
router.post("/", async (req, res) => {
  const { name, description, is_in_use, is_active } = req.body;

  try {
    const query = `
      INSERT INTO machines (name, description, is_in_use, is_active) 
      VALUES ($1, $2, $3, $4) 
      RETURNING *;
    `;
    const values = [name, description, is_in_use ?? false, is_active ?? true];

    const result = await pool.query(query, values);

    res.status(201).json({
      message: "Máquina creada exitosamente",
      machine: result.rows[0],
    });
  } catch (error) {
    console.error("Error al crear máquina:", error.message);
    res.status(500).json({ error: "Error al crear la máquina" });
  }
});

// Eliminar una máquina
router.delete("/:id", async (req, res) => {
  const { id } = req.params;

  try {
    const query = "DELETE FROM machines WHERE id = $1 RETURNING *;";
    const values = [id];

    const result = await pool.query(query, values);

    if (result.rows.length === 0) {
      return res.status(404).json({ error: "Máquina no encontrada" });
    }

    res.status(200).json({
      message: "Máquina eliminada exitosamente",
      machine: result.rows[0],
    });
  } catch (error) {
    console.error("Error al eliminar máquina:", error.message);
    res.status(500).json({ error: "Error al eliminar la máquina" });
  }
});

// Actualizar una máquina (PATCH)
router.patch("/:id", async (req, res) => {
  const { id } = req.params;
  const { name, description, is_in_use, is_active } = req.body;

  try {
    // Crear dinámicamente las columnas a actualizar
    const updates = [];
    const values = [];
    let index = 1;

    if (name !== undefined) {
      updates.push(`name = $${index}`);
      values.push(name);
      index++;
    }
    if (description !== undefined) {
      updates.push(`description = $${index}`);
      values.push(description);
      index++;
    }
    if (is_in_use !== undefined) {
      updates.push(`is_in_use = $${index}`);
      values.push(is_in_use);
      index++;
    }
    if (is_active !== undefined) {
      updates.push(`is_active = $${index}`);
      values.push(is_active);
      index++;
    }

    if (updates.length === 0) {
      return res.status(400).json({ error: "No hay campos para actualizar" });
    }

    // Agregar la cláusula de actualización de timestamp
    updates.push(`updated_at = NOW()`);

    // Agregar ID como último valor
    values.push(id);

    // Construir la consulta de actualización
    const query = `
      UPDATE machines 
      SET ${updates.join(", ")} 
      WHERE id = $${index}
      RETURNING *;
    `;

    const result = await pool.query(query, values);

    if (result.rows.length === 0) {
      return res.status(404).json({ error: "Máquina no encontrada" });
    }

    res.status(200).json({
      message: "Máquina actualizada exitosamente",
      machine: result.rows[0],
    });
  } catch (error) {
    console.error("Error al actualizar máquina:", error.message);
    res.status(500).json({ error: "Error al actualizar la máquina" });
  }
});

// Otros endpoints de máquinas (si es necesario):
// - Obtener todas las máquinas
// - Crear una máquina nueva
// - Eliminar una máquina

export default router;
