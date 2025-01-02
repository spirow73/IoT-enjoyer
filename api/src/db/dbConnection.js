import pkg from "pg";
const { Pool } = pkg;

const pool = new Pool({
  user: "postgres",
  host: "host.docker.internal", // Apunta al host de tu máquina
  database: "gym_db",
  password: "postgres",
  port: 5432,
});

export default pool;
