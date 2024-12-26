const express = require('express');
const routes = require('./routes');
const app = express();
const port = 3000;

app.use(express.json()); // Para que acepte el body en formato JSON

// Usamos la configuración de rutas definida en routes.js
app.use('/users', routes);


app.listen(port, () => {
    console.log(`User service running on port ${port}`);
});