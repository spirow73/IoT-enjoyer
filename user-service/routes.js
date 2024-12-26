const express = require('express');
const router = express.Router();

// Datos de prueba (simulando la base de datos)
const users = [
    { id: 1, rfid: "rfid123", name: "Usuario 1" },
    { id: 2, rfid: "rfid456", name: "Usuario 2" },
];

// Endpoint para obtener todos los usuarios
router.get('/', (req, res) => {
    res.json(users);
});

// Endpoint para obtener un usuario por su ID
router.get('/:id', (req, res) => {
    const userId = parseInt(req.params.id);
    const user = users.find(u => u.id === userId);
    if (user) {
        res.json(user);
    } else {
        res.status(404).send('Usuario no encontrado');
    }
});

// Endpoint para agregar un usuario
router.post('/', (req, res) => {
    const newUser = {
        id: users.length + 1,
        rfid: req.body.rfid,
        name: req.body.name
      };
      users.push(newUser);
      res.status(201).json(newUser);
  });


module.exports = router;