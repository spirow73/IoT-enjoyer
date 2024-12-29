import express from "express";
import pool from "../db/dbConnection.js";

const router = express.Router();

// Crear un usuario
router.post("/", async (req, res) => {
  const { first_name, last_name, username, rfid_tag } = req.body;

  try {
    const query = `
      INSERT INTO users (first_name, last_name, username, rfid_tag) 
      VALUES ($1, $2, $3, $4) 
      RETURNING *;
    `;
    const values = [first_name, last_name, username, rfid_tag];

    const result = await pool.query(query, values);

    res.status(201).json({
      message: "Usuario creado exitosamente",
      user: result.rows[0],
    });
  } catch (error) {
    console.error("Error al crear usuario:", error.message);
    res.status(500).json({ error: "Error al crear el usuario" });
  }
});

// Obtener todos los usuarios
router.get("/", async (req, res) => {
  try {
    const query = "SELECT * FROM users;";
    const result = await pool.query(query);

    res.status(200).json(result.rows);
  } catch (error) {
    console.error("Error al obtener usuarios:", error.message);
    res.status(500).json({ error: "Error al obtener los usuarios" });
  }
});

// Obtener un usuario por ID
router.get("/:id", async (req, res) => {
  const { id } = req.params;

  try {
    const query = "SELECT * FROM users WHERE id = $1;";
    const values = [id];
    const result = await pool.query(query, values);

    if (result.rows.length === 0) {
      return res.status(404).json({ error: "Usuario no encontrado" });
    }

    res.status(200).json(result.rows[0]);
  } catch (error) {
    console.error("Error al obtener usuario:", error.message);
    res.status(500).json({ error: "Error al obtener el usuario" });
  }
});

// Actualizar un usuario
// Actualizar un usuario (PATCH)
router.patch("/:id", async (req, res) => {
    const { id } = req.params;
    const { first_name, last_name, username, rfid_tag } = req.body;
  
    try {
      // Crear dinámicamente las columnas a actualizar
      const updates = [];
      const values = [];
      let index = 1;
  
      if (first_name !== undefined) {
        updates.push(`first_name = $${index}`);
        values.push(first_name);
        index++;
      }
      if (last_name !== undefined) {
        updates.push(`last_name = $${index}`);
        values.push(last_name);
        index++;
      }
      if (username !== undefined) {
        updates.push(`username = $${index}`);
        values.push(username);
        index++;
      }
      if (rfid_tag !== undefined) {
        updates.push(`rfid_tag = $${index}`);
        values.push(rfid_tag);
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
        UPDATE users 
        SET ${updates.join(", ")} 
        WHERE id = $${index}
        RETURNING *;
      `;
  
      const result = await pool.query(query, values);
  
      if (result.rows.length === 0) {
        return res.status(404).json({ error: "Usuario no encontrado" });
      }
  
      res.status(200).json({
        message: "Usuario actualizado exitosamente",
        user: result.rows[0],
      });
    } catch (error) {
      console.error("Error al actualizar usuario:", error.message);
      res.status(500).json({ error: "Error al actualizar el usuario" });
    }
  });

// Eliminar un usuario
router.delete("/:id", async (req, res) => {
  const { id } = req.params;

  try {
    const query = "DELETE FROM users WHERE id = $1 RETURNING *;";
    const values = [id];
    const result = await pool.query(query, values);

    if (result.rows.length === 0) {
      return res.status(404).json({ error: "Usuario no encontrado" });
    }

    res.status(200).json({
      message: "Usuario eliminado exitosamente",
      user: result.rows[0],
    });
  } catch (error) {
    console.error("Error al eliminar usuario:", error.message);
    res.status(500).json({ error: "Error al eliminar el usuario" });
  }
});

export default router;
