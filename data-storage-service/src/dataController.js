import db from '../db.js';

export const getData = async (req, res) => {
  try {
    const result = await db.query('SELECT * FROM your_table_name');
    res.json(result.rows);
  } catch (error) {
    console.error(error);
    res.status(500).json({ error: 'Error al obtener datos' });
  }
};

export const addData = async (req, res) => {
  try {
    const { field1, field2 } = req.body;
    const result = await db.query(
      'INSERT INTO your_table_name (field1, field2) VALUES ($1, $2) RETURNING *',
      [field1, field2]
    );
    res.status(201).json(result.rows[0]);
  } catch (error) {
    console.error(error);
    res.status(500).json({ error: 'Error al agregar datos' });
  }
};
