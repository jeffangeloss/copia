export interface LocalUser {
  id: string;
  email: string;
  name: string;
  role: "Operador" | "Administrador" | "Invitado";
  area: string;
  passwordHash: string;
}

function hashPassword(input: string): string {
  let hash = 0x811c9dc5;
  for (let i = 0; i < input.length; i++) {
    hash ^= input.charCodeAt(i);
    hash = Math.imul(hash, 0x01000193);
    hash >>>= 0;
  }
  return `fnv1a-${hash.toString(16).padStart(8, "0")}`;
}

const seededUsers: LocalUser[] = [
  {
    id: "op-01",
    email: "ana.ramirez@trafico.local",
    name: "Ana Ramírez",
    role: "Operador",
    area: "Centro Histórico",
    passwordHash: hashPassword("Semaforo#01"),
  },
  {
    id: "op-02",
    email: "ricardo.sosa@trafico.local",
    name: "Ricardo Sosa",
    role: "Operador",
    area: "Zona Norte",
    passwordHash: hashPassword("Semaforo#02"),
  },
  {
    id: "op-03",
    email: "marcela.arias@trafico.local",
    name: "Marcela Arias",
    role: "Operador",
    area: "Zona Industrial",
    passwordHash: hashPassword("Semaforo#03"),
  },
  {
    id: "op-04",
    email: "fernando.lagos@trafico.local",
    name: "Fernando Lagos",
    role: "Operador",
    area: "Zona Universitaria",
    passwordHash: hashPassword("Semaforo#04"),
  },
  {
    id: "op-05",
    email: "claudia.vargas@trafico.local",
    name: "Claudia Vargas",
    role: "Operador",
    area: "Zona Sur",
    passwordHash: hashPassword("Semaforo#05"),
  },
  {
    id: "op-06",
    email: "gustavo.pena@trafico.local",
    name: "Gustavo Peña",
    role: "Operador",
    area: "Anillo Periférico",
    passwordHash: hashPassword("Semaforo#06"),
  },
  {
    id: "adm-01",
    email: "maria.ortega@trafico.local",
    name: "María Ortega",
    role: "Administrador",
    area: "Coordinación General",
    passwordHash: hashPassword("Admin#2024"),
  },
  {
    id: "adm-02",
    email: "hector.paredes@trafico.local",
    name: "Héctor Paredes",
    role: "Administrador",
    area: "Seguridad Vial",
    passwordHash: hashPassword("Admin#2025"),
  },
  {
    id: "inv-01",
    email: "inspectora.navarro@trafico.local",
    name: "Inspectora Navarro",
    role: "Invitado",
    area: "Supervisión",
    passwordHash: hashPassword("Invitado#1"),
  },
  {
    id: "inv-02",
    email: "ingeniero.salas@trafico.local",
    name: "Ing. Salas",
    role: "Invitado",
    area: "Infraestructura",
    passwordHash: hashPassword("Invitado#2"),
  },
];

export function verifyLocalUser(
  email: string,
  password: string
): Omit<LocalUser, "passwordHash"> | null {
  const normalizedEmail = email.trim().toLowerCase();
  const passwordHash = hashPassword(password);
  const found = seededUsers.find((user) => user.email === normalizedEmail);
  if (!found) {
    return null;
  }
  if (found.passwordHash !== passwordHash) {
    return null;
  }
  const { passwordHash: _hash, ...safeUser } = found;
  return safeUser;
}

export function listLocalUsers(): Omit<LocalUser, "passwordHash">[] {
  return seededUsers.map(({ passwordHash, ...rest }) => rest);
}
