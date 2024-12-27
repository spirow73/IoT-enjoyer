import express from 'express';

import routes from './routes.js'

const app = express();
const port = 3000;

app.use(express.json()); // Para que acepte el body en formato JSON

// Usamos la configuración de rutas definida en routes.js
app.use('/users', routes);

app.listen(port, () => {
    console.log(`User service running on port ${port}`);
});