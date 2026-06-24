-- ─── CLIENTS (agencies / orgs that own or buy tags) ─────────────────────────
-- An agency that purchases tags (e.g. a medical service or a police dept). Used
-- to keep each agency's data isolated: a tag is mapped to its client in the
-- device registry, and every event is stamped with that client_id server-side
-- (see the trigger below). The client_id therefore never has to travel over the
-- air — the tag transmits only its tag_id and the backend resolves the owner.
create table clients (
  client_id   uuid primary key default gen_random_uuid(),
  name        text not null,
  created_at  timestamptz default now()
);

-- Pointage itself: the operator/manufacturer, and the default owner+client for
-- tags that haven't been assigned to a purchasing agency yet. Fixed UUID so it
-- can be referenced as a column default below.
insert into clients (client_id, name)
  values ('00000000-0000-0000-0000-000000000001', 'Pointage');

-- ─── DEVICES ────────────────────────────────────────────────────────────────
create table devices (
  id           uuid primary key default gen_random_uuid(),
  tag_id       text unique not null,
  role         text not null default 'tag',        -- 'tag' | 'receiver'
  lifecycle    text not null default 'provisioned', -- see constraint below
  label        text,
  -- owner_id  = the org that operates/issued the tag (Pointage for now).
  -- client_id = the agency that bought it; drives per-agency data isolation.
  -- Both default to Pointage until a tag is assigned to a purchasing agency.
  owner_id     uuid references clients(client_id)
                 default '00000000-0000-0000-0000-000000000001',
  client_id    uuid references clients(client_id)
                 default '00000000-0000-0000-0000-000000000001',
  last_seen    timestamptz,
  created_at   timestamptz default now()
);

alter table devices
  add constraint lifecycle_values
  check (lifecycle in ('provisioned','deployed','active','used','retired'));

-- ─── EVENTS ─────────────────────────────────────────────────────────────────
create table events (
  id            uuid primary key default gen_random_uuid(),
  device_id     uuid references devices(id),
  -- client_id is denormalized here from the device registry by the trigger
  -- below (NOT sent by the tag). Lets row-level security filter each agency's
  -- events with a single indexed predicate.
  client_id     uuid references clients(client_id),
  tag_id        text not null,
  seq           integer,
  type          text,
  torn          smallint,
  level         smallint,
  tag_status    text,
  triage_status text,
  disposition   text,
  battery_pct   smallint,
  rssi          smallint,
  gps_lat       double precision,
  gps_lon       double precision,
  gps_accuracy_m real,
  gps_fix       boolean,
  nfc_uid       text,
  nfc_payload   text,
  received_at   timestamptz,            -- clock from payload
  inserted_at   timestamptz default now()  -- db clock
);

create unique index on events (tag_id, seq);   -- dedup key
create index on events (tag_id, received_at desc);
create index on events (received_at desc);
create index on events (triage_status);
create index on events (client_id, received_at desc);  -- per-agency queries / RLS

-- ─── TRIGGER: auto-provision device, keep last_seen current, stamp client ────
create or replace function upsert_device_on_event()
returns trigger language plpgsql as $$
begin
  insert into devices (tag_id, role, lifecycle, last_seen)
  values (NEW.tag_id, 'tag', 'provisioned', NEW.inserted_at)
  on conflict (tag_id) do update
    set last_seen = excluded.last_seen,
        -- bump lifecycle: deployed → active when tag says so
        lifecycle = case
          when devices.lifecycle = 'deployed' and NEW.tag_status = 'active'
          then 'active'
          else devices.lifecycle
        end;

  -- Enrich the event from the device registry: attach device_id and the owning
  -- agency's client_id. This is the server side of "client_id never goes over
  -- LoRa" — the tag sends only tag_id, the client is resolved here.
  update events e
    set device_id = d.id,
        client_id = d.client_id
  from devices d
  where d.tag_id = NEW.tag_id
    and e.id = NEW.id;

  return NEW;
end;
$$;

create trigger trg_upsert_device
  after insert on events
  for each row execute function upsert_device_on_event();

-- ─── PER-AGENCY ISOLATION (row-level security) ───────────────────────────────
-- Isolation is enforced here, in the database, NOT on the radio link (RF is
-- broadcast — a receiver in range hears every agency's tags regardless of
-- payload). Enable RLS and scope each agency to its own client_id once the
-- app's auth maps a logged-in user to a client. Sketch:
--
--   alter table events  enable row level security;
--   alter table devices enable row level security;
--   create policy events_by_client on events
--     using (client_id = current_setting('app.client_id')::uuid);
--
-- Left disabled here so local/dev ingestion keeps working without auth wiring.
